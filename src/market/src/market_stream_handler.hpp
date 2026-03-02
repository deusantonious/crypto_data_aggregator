#pragma once

#include <functional>
#include <market/i_market_stream_handler.hpp>
#include <memory>
#include <optional>

#include "interfaces/i_exchange_connection_manager.hpp"

namespace market {

/// @brief Concrete market stream handler that delegates connection management and
///        message parsing through injected strategies.
class MarketStreamHandler : public IMarketStreamHandler {
public:
    using ParseMessageStrategy = std::function<std::optional<TradeEvent>(const std::string&)>;

    /// @brief Constructs a handler with the given connection manager and message parser.
    /// @param connectionMgr Shared connection manager that handles the WebSocket lifecycle.
    /// @param parseMessage  Strategy that converts a raw exchange message into a TradeEvent.
    MarketStreamHandler(std::shared_ptr<IExchangeConnectionManager> connectionMgr,
                        const ParseMessageStrategy& parseMessage);

    void start() override;
    void stop() override;
    void subscribeOnTrade(TradeCallback callback) override;

private:
    /// @brief Processes a raw message by parsing it and forwarding valid trades to the subscriber.
    /// @param raw The raw JSON message received from the exchange.
    void onRawMessage(const std::string& raw);

    std::shared_ptr<IExchangeConnectionManager> m_connectionMgr;
    ParseMessageStrategy m_parseMessage;
    TradeCallback m_tradeCallback;
};

}  // namespace market
