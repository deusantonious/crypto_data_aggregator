#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>

#include "i_market_stream_handler.hpp"

namespace market {

enum class Exchange : std::uint8_t { Binance };

enum class CurrencyPair : std::uint8_t { BTCUSDT, ETHUSDT, BNBUSDT, XRPUSDT, SOLUSDT };

/// Creates, owns, and manages the lifecycle of market stream handlers.
class MarketStreamsController {
public:
    MarketStreamsController();
    ~MarketStreamsController();

    MarketStreamsController(const MarketStreamsController&) = delete;
    MarketStreamsController& operator=(const MarketStreamsController&) = delete;

    MarketStreamsController(MarketStreamsController&&) noexcept;
    MarketStreamsController& operator=(MarketStreamsController&&) noexcept;

    /// Creates a handler for the given exchange and currency pairs.
    /// The controller owns the handler. Returns a reference for subscribing.
    /// @throws std::invalid_argument if the exchange is not supported.
    std::shared_ptr<IMarketStreamHandler> addHandler(Exchange exchange,
                                                     std::span<const CurrencyPair> pairs);

    /// Starts all registered handlers (connects to exchanges).
    void startAll();

    /// Stops all handlers (disconnects from exchanges).
    void stopAll();

    /// Returns true if at least one handler has been added.
    [[nodiscard]] bool hasHandlers() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace market
