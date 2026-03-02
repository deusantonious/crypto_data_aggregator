#include "binance_connection_manager.hpp"

#include <spdlog/spdlog.h>

#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/beast/core/buffers_to_string.hpp>

namespace market {

BinanceConnectionManager::BinanceConnectionManager(boost::asio::io_context& ioContext,
                                                   std::string host, std::string port,
                                                   std::string path)
    : m_ioContext(ioContext),
      m_sslCtx(boost::asio::ssl::context::tlsv12_client),
      m_resolver(ioContext),
      m_reconnect(ioContext),
      m_host(std::move(host)),
      m_port(std::move(port)),
      m_path(std::move(path)) {
    m_sslCtx.set_default_verify_paths();
    m_sslCtx.set_verify_mode(boost::asio::ssl::verify_peer);
}

BinanceConnectionManager::~BinanceConnectionManager() {
    try {
        disconnect();
    } catch (...) {
        spdlog::warn("Unexpected exception during BinanceConnectionManager shutdown");
    }
}

template <typename... Args>
auto BinanceConnectionManager::weakBind(void (BinanceConnectionManager::*method)(Args...)) {
    return [weak = weak_from_this(), method](Args... args) {
        if (auto self = weak.lock()) {
            (self.get()->*method)(args...);
        }
    };
}

void BinanceConnectionManager::connect() {
    if (m_state != State::Disconnected) {
        return;
    }

    spdlog::info("Connecting to {}:{}{}", m_host, m_port, m_path);
    doResolve();
}

void BinanceConnectionManager::disconnect() {
    const bool wasConnected = (m_state == State::Connected);
    m_state = State::Disconnecting;
    m_reconnect.timer().cancel();
    m_resolver.cancel();

    if (m_ws && wasConnected) {
        boost::system::error_code ec;
        m_ws->close(boost::beast::websocket::close_code::normal, ec);
        if (ec) {
            spdlog::warn("Error closing WebSocket: {}", ec.message());
        }
    }

    m_state = State::Disconnected;
    m_ws.reset();
}

void BinanceConnectionManager::setMessageCallback(MessageCallback callback) {
    m_messageCallback = std::move(callback);
}

bool BinanceConnectionManager::isConnected() const {
    return m_state == State::Connected;
}

void BinanceConnectionManager::doResolve() {
    m_state = State::Connecting;
    m_resolver.async_resolve(m_host, m_port, weakBind(&BinanceConnectionManager::onResolve));
}

void BinanceConnectionManager::onResolve(
    boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type& results) {
    if (ec) {
        onFail(ec, "resolve");
        return;
    }

    m_ws = std::make_unique<WsStream>(m_ioContext, m_sslCtx);
    boost::beast::get_lowest_layer(*m_ws).expires_after(std::chrono::seconds(30));
    boost::beast::get_lowest_layer(*m_ws).async_connect(
        results, weakBind(&BinanceConnectionManager::onConnect));
}

void BinanceConnectionManager::onConnect(
    boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type::endpoint_type&
    /*endpoint*/) {
    if (ec) {
        onFail(ec, "connect");
        return;
    }

    boost::beast::get_lowest_layer(*m_ws).expires_after(std::chrono::seconds(30));

    if (!configureSsl()) {
        return;
    }

    m_ws->next_layer().async_handshake(boost::asio::ssl::stream_base::client,
                                       weakBind(&BinanceConnectionManager::onSslHandshake));
}

bool BinanceConnectionManager::configureSsl() {
    if (!SSL_set_tlsext_host_name(m_ws->next_layer().native_handle(), m_host.c_str())) {
        auto ec = boost::system::error_code(static_cast<int>(::ERR_get_error()),
                                            boost::asio::error::get_ssl_category());
        onFail(ec, "ssl_set_hostname");
        return false;
    }

    m_ws->next_layer().set_verify_callback(boost::asio::ssl::host_name_verification(m_host));
    return true;
}

void BinanceConnectionManager::onSslHandshake(boost::system::error_code ec) {
    if (ec) {
        onFail(ec, "ssl_handshake");
        return;
    }

    boost::beast::get_lowest_layer(*m_ws).expires_never();

    auto timeout =
        boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client);
    timeout.keep_alive_pings = true;
    m_ws->set_option(timeout);

    m_ws->read_message_max(kMaxMessageSizeBytes);

    m_ws->set_option(boost::beast::websocket::stream_base::decorator(
        [](boost::beast::websocket::request_type& req) {
            req.set(boost::beast::http::field::user_agent, "crypto_data_aggregator");
        }));

    m_ws->async_handshake(m_host, m_path, weakBind(&BinanceConnectionManager::onWsHandshake));
}

void BinanceConnectionManager::onWsHandshake(boost::system::error_code ec) {
    if (ec) {
        onFail(ec, "ws_handshake");
        return;
    }

    m_state = State::Connected;
    m_reconnect.reset();
    spdlog::info("WebSocket connected to {}", m_host);

    doRead();
}

void BinanceConnectionManager::doRead() {
    m_ws->async_read(m_buffer, weakBind(&BinanceConnectionManager::onRead));
}

void BinanceConnectionManager::onRead(boost::system::error_code ec,
                                      std::size_t /*bytesTransferred*/) {
    if (ec) {
        if (ec == boost::beast::websocket::error::closed) {
            spdlog::info("WebSocket closed by server");
        }
        handleDisconnection();
        return;
    }

    std::string msg = boost::beast::buffers_to_string(m_buffer.data());
    m_buffer.consume(m_buffer.size());

    if (m_messageCallback) {
        m_messageCallback(msg);
    }

    doRead();
}

void BinanceConnectionManager::onFail(boost::system::error_code ec, const char* what) {
    spdlog::error("WebSocket {}: {}", what, ec.message());
    handleDisconnection();
}

void BinanceConnectionManager::handleDisconnection() {
    if (m_state == State::Disconnecting) {
        m_state = State::Disconnected;
        return;
    }

    m_state = State::Disconnected;
    scheduleReconnect();
}

void BinanceConnectionManager::onReconnectTimer(boost::system::error_code ec) {
    if (ec || m_state == State::Disconnecting) {
        return;
    }

    m_reconnect.escalate();
    doResolve();
}

void BinanceConnectionManager::scheduleReconnect() {
    const auto reconnectDelay = m_reconnect.nextDelay();

    spdlog::info("Reconnecting in {} ms...",
                 std::chrono::duration_cast<std::chrono::milliseconds>(reconnectDelay).count());

    m_ws.reset();

    m_reconnect.timer().expires_after(reconnectDelay);
    m_reconnect.timer().async_wait(weakBind(&BinanceConnectionManager::onReconnectTimer));
}

}  // namespace market
