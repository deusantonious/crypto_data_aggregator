#include "market_stream_handler.hpp"

#include <utility>

namespace market {

MarketStreamHandler::MarketStreamHandler(std::shared_ptr<IExchangeConnectionManager> connectionMgr,
                                         const ParseMessageStrategy& parseMessage)
    : m_connectionMgr(std::move(connectionMgr)), m_parseMessage(parseMessage) {}

void MarketStreamHandler::start() {
    m_connectionMgr->setMessageCallback([this](const std::string& msg) { onRawMessage(msg); });

    m_connectionMgr->connect();
}

void MarketStreamHandler::stop() {
    m_connectionMgr->setMessageCallback(nullptr);
    m_connectionMgr->disconnect();
}

void MarketStreamHandler::subscribeOnTrade(TradeCallback callback) {
    m_tradeCallback = std::move(callback);
}

void MarketStreamHandler::onRawMessage(const std::string& raw) {
    auto event = m_parseMessage(raw);
    if (event && m_tradeCallback) {
        m_tradeCallback(*event);
    }
}

}  // namespace market
