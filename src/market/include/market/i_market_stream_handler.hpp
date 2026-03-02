#pragma once

#include <functional>

#include "trade_event.hpp"

namespace market {

class IMarketStreamHandler {
public:
    using TradeCallback = std::function<void(const TradeEvent&)>;

    virtual ~IMarketStreamHandler() = default;
    IMarketStreamHandler(const IMarketStreamHandler&) = delete;
    IMarketStreamHandler& operator=(const IMarketStreamHandler&) = delete;
    IMarketStreamHandler(IMarketStreamHandler&&) = delete;
    IMarketStreamHandler& operator=(IMarketStreamHandler&&) = delete;

    /// @brief Starts the market data stream (connects to the exchange).
    virtual void start() = 0;

    /// @brief Stops the market data stream (disconnects from the exchange).
    virtual void stop() = 0;

    /// @brief Registers a callback invoked for each incoming trade event.
    /// @param callback Called with a TradeEvent for every trade received on the stream.
    virtual void subscribeOnTrade(TradeCallback callback) = 0;

protected:
    IMarketStreamHandler() = default;
};

}  // namespace market
