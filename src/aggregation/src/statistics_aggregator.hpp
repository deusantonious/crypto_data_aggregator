#pragma once

#include <aggregation/i_statistics_aggregator.hpp>
#include <chrono>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "window_stats.hpp"

namespace aggregation {

/// @brief Concrete statistics aggregator that accumulates trades per symbol over
///        fixed-length time windows and notifies subscribers when each window elapses.
class StatisticsAggregator : public IStatisticsAggregator {
public:
    /// @brief Constructs an aggregator with the given window duration.
    /// @param windowDuration Length of each aggregation window. Clamped to a minimum of 1 second.
    explicit StatisticsAggregator(std::chrono::seconds windowDuration);
    ~StatisticsAggregator() override = default;

    StatisticsAggregator(const StatisticsAggregator&) = delete;
    StatisticsAggregator& operator=(const StatisticsAggregator&) = delete;
    StatisticsAggregator(StatisticsAggregator&&) = delete;
    StatisticsAggregator& operator=(StatisticsAggregator&&) = delete;

    void start() override;
    void stop() override;

    void addTrade(const market::TradeEvent& event) override;
    void subscribeOnWindowElapsed(WindowElapsedCallback callback) override;

private:
    /// @brief Runs the periodic window loop, sleeping for m_windowDuration between snapshots.
    /// @param stopToken Token checked between iterations to allow graceful shutdown.
    void runWindowLoop(const std::stop_token& stopToken);

    /// @brief Atomically captures all current per-symbol stats and resets them for the next window.
    /// @return A vector of SymbolSnapshot objects for every symbol seen since the last reset.
    std::vector<SymbolSnapshot> snapshotAndReset();

    /// @brief Invokes all registered window-elapsed callbacks with the given snapshots.
    /// @param snapshots The per-symbol statistics from the completed window.
    void notifyWindowElapsed(const std::vector<SymbolSnapshot>& snapshots);

    const std::chrono::seconds m_windowDuration;

    mutable std::mutex m_tradesMutex;
    mutable std::mutex m_callbacksMutex;
    std::jthread m_windowThread;
    std::unordered_map<std::string, WindowStats> m_windows;
    std::vector<WindowElapsedCallback> m_windowElapsedCallbacks;
};

}  // namespace aggregation
