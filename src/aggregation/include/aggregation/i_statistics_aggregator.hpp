#pragma once

#include <functional>
#include <market/trade_event.hpp>
#include <string>
#include <vector>

#include "symbol_snapshot.hpp"

namespace aggregation {

class IStatisticsAggregator {
public:
    using WindowElapsedCallback = std::function<void(const std::vector<SymbolSnapshot>&)>;

    virtual ~IStatisticsAggregator() = default;
    IStatisticsAggregator(const IStatisticsAggregator&) = delete;
    IStatisticsAggregator& operator=(const IStatisticsAggregator&) = delete;
    IStatisticsAggregator(IStatisticsAggregator&&) = delete;
    IStatisticsAggregator& operator=(IStatisticsAggregator&&) = delete;

    /// @brief Starts the periodic aggregation window loop.
    virtual void start() = 0;

    /// @brief Stops the aggregation window loop and releases its thread.
    virtual void stop() = 0;

    /// @brief Records a single trade into the current aggregation window.
    /// @param event The trade event containing symbol, price, quantity, and side.
    virtual void addTrade(const market::TradeEvent& event) = 0;

    /// @brief Registers a callback invoked each time an aggregation window elapses.
    /// @param callback Called with a vector of per-symbol snapshots for the completed window.
    virtual void subscribeOnWindowElapsed(WindowElapsedCallback callback) = 0;

protected:
    IStatisticsAggregator() = default;
};

}  // namespace aggregation
