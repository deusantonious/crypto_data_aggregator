#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <string>

#include "../interfaces/i_exchange_connection_manager.hpp"

namespace market {

/// @brief WebSocket connection manager for the Binance exchange.
///        Handles TLS+WebSocket handshake, automatic reconnection with exponential backoff,
///        and asynchronous message dispatch via Boost.Beast.
class BinanceConnectionManager final
    : public IExchangeConnectionManager,
      public std::enable_shared_from_this<BinanceConnectionManager> {
public:
    /// @brief Constructs a connection manager bound to the given ASIO context.
    /// @param ioContext The Boost.Asio I/O context used for all async operations.
    /// @param host      WebSocket server hostname (e.g. "stream.binance.com").
    /// @param port      WebSocket server port (e.g. "9443").
    /// @param path      WebSocket request path including stream subscriptions.
    BinanceConnectionManager(boost::asio::io_context& ioContext, std::string host, std::string port,
                             std::string path);

    ~BinanceConnectionManager() override;

    BinanceConnectionManager(const BinanceConnectionManager&) = delete;
    BinanceConnectionManager& operator=(const BinanceConnectionManager&) = delete;
    BinanceConnectionManager(BinanceConnectionManager&&) = delete;
    BinanceConnectionManager& operator=(BinanceConnectionManager&&) = delete;

    void connect() override;
    void disconnect() override;
    void setMessageCallback(MessageCallback callback) override;
    bool isConnected() const override;

private:
    enum class State : std::uint8_t {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting,
    };

    using SslStream = boost::beast::ssl_stream<boost::beast::tcp_stream>;
    using WsStream = boost::beast::websocket::stream<SslStream>;

    template <typename... Args>
    auto weakBind(void (BinanceConnectionManager::*method)(Args...));

    /// @brief Initiates async DNS resolution for the target host.
    void doResolve();

    /// @brief Callback after DNS resolution; opens a TCP connection to the resolved endpoint.
    void onResolve(boost::system::error_code ec,
                   const boost::asio::ip::tcp::resolver::results_type& results);

    /// @brief Callback after TCP connect; starts the TLS handshake.
    void onConnect(boost::system::error_code ec,
                   const boost::asio::ip::tcp::resolver::results_type::endpoint_type& endpoint);

    /// @brief Configures SNI hostname and certificate verification for the TLS layer.
    /// @return True on success, false if SNI setup fails (triggers onFail).
    [[nodiscard]] bool configureSsl();

    /// @brief Callback after TLS handshake; initiates the WebSocket upgrade.
    void onSslHandshake(boost::system::error_code ec);

    /// @brief Callback after WebSocket upgrade; transitions to Connected state and starts reading.
    void onWsHandshake(boost::system::error_code ec);

    /// @brief Posts an async read for the next WebSocket message.
    void doRead();

    /// @brief Callback for each received WebSocket message; dispatches to the message callback.
    void onRead(boost::system::error_code ec, std::size_t bytesTransferred);

    /// @brief Logs an error and transitions to the disconnected/reconnecting state.
    /// @param ec   The error code from the failed operation.
    /// @param what Human-readable label for the failed operation stage.
    void onFail(boost::system::error_code ec, const char* what);

    /// @brief Handles an unexpected disconnection by scheduling a reconnect attempt.
    void handleDisconnection();

    /// @brief Callback fired when the reconnect timer expires; escalates delay and reconnects.
    void onReconnectTimer(boost::system::error_code ec);

    /// @brief Schedules a reconnection attempt with exponential backoff and jitter.
    void scheduleReconnect();

    /// @brief Exponential backoff reconnection policy with random jitter.
    struct ReconnectPolicy {
        static constexpr std::chrono::seconds kMaxDelay{30};  ///< Upper bound on backoff delay.
        static constexpr std::chrono::milliseconds kMaxJitter{
            250};  ///< Maximum random jitter added to each delay.

        explicit ReconnectPolicy(boost::asio::io_context& ioContext) : m_timer(ioContext) {}

        /// @brief Computes the next reconnect delay (current backoff + random jitter).
        /// @return The total delay before the next reconnection attempt.
        [[nodiscard]] std::chrono::milliseconds nextDelay() {
            return m_delay + std::chrono::milliseconds(m_jitterDistribution(m_jitterGenerator));
        }

        /// @brief Doubles the backoff delay, capped at kMaxDelay.
        void escalate() {
            m_delay = std::min(m_delay * 2, kMaxDelay);
        }

        /// @brief Resets the backoff delay to the initial 1-second value.
        void reset() {
            m_delay = std::chrono::seconds{1};
        }

        boost::asio::steady_timer& timer() {
            return m_timer;
        }

    private:
        boost::asio::steady_timer m_timer;
        std::chrono::seconds m_delay{1};
        std::mt19937 m_jitterGenerator{std::random_device{}()};
        std::uniform_int_distribution<int> m_jitterDistribution{
            0, static_cast<int>(kMaxJitter.count())};
    };

    // IO resources
    boost::asio::io_context& m_ioContext;
    boost::asio::ssl::context m_sslCtx;
    boost::asio::ip::tcp::resolver m_resolver;
    std::unique_ptr<WsStream> m_ws;
    boost::beast::flat_buffer m_buffer;

    // Connection identity
    std::string m_host;
    std::string m_port;
    std::string m_path;

    // State
    State m_state{State::Disconnected};
    ReconnectPolicy m_reconnect;
    static constexpr std::size_t kMaxMessageSizeBytes =
        static_cast<std::size_t>(1024) * static_cast<std::size_t>(1024);

    MessageCallback m_messageCallback;
};

}  // namespace market
