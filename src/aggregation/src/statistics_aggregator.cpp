#include "statistics_aggregator.hpp"

#include <condition_variable>
#include <utility>

namespace aggregation {

StatisticsAggregator::StatisticsAggregator(std::chrono::seconds windowDuration)
    : m_windowDuration(windowDuration <= std::chrono::seconds::zero() ? std::chrono::seconds(1)
                                                                      : windowDuration) {}

void StatisticsAggregator::start() {
    if (m_windowThread.joinable()) {
        return;
    }

    m_windowThread =
        std::jthread([this](const std::stop_token& stopToken) { runWindowLoop(stopToken); });
}

void StatisticsAggregator::stop() {
    m_windowThread.request_stop();
    if (m_windowThread.joinable()) {
        m_windowThread.join();
    }
}

void StatisticsAggregator::subscribeOnWindowElapsed(WindowElapsedCallback callback) {
    std::lock_guard lock(m_callbacksMutex);
    m_windowElapsedCallbacks.push_back(std::move(callback));
}

std::vector<SymbolSnapshot> StatisticsAggregator::snapshotAndReset() {
    std::vector<SymbolSnapshot> snapshots;
    {
        std::lock_guard lock(m_tradesMutex);

        snapshots.reserve(m_windows.size());

        for (auto& [symbol, stats] : m_windows) {
            snapshots.push_back(stats.toSnapshot(symbol));
            stats.reset();
        }
    }

    return snapshots;
}

void StatisticsAggregator::notifyWindowElapsed(const std::vector<SymbolSnapshot>& snapshots) {
    std::vector<WindowElapsedCallback> callbacks;
    {
        std::lock_guard lock(m_callbacksMutex);
        callbacks = m_windowElapsedCallbacks;
    }

    for (const auto& callback : callbacks) {
        callback(snapshots);
    }
}

void StatisticsAggregator::runWindowLoop(const std::stop_token& stopToken) {
    std::mutex sleepMutex;
    // Using condition variable to allow interrupting sleep when stop is requested
    // This variable is not used for synchronization of any shared data
    std::condition_variable sleepCv;

    std::stop_callback stopCb(stopToken, [&sleepCv] { sleepCv.notify_all(); });

    while (!stopToken.stop_requested()) {
        std::unique_lock lock(sleepMutex);
        sleepCv.wait_for(lock, m_windowDuration,
                         [&stopToken] { return stopToken.stop_requested(); });
        lock.unlock();

        if (stopToken.stop_requested()) {
            break;
        }

        auto snapshots = snapshotAndReset();
        if (!snapshots.empty()) {
            notifyWindowElapsed(snapshots);
        }
    }
}

void StatisticsAggregator::addTrade(const market::TradeEvent& event) {
    std::lock_guard lock(m_tradesMutex);
    m_windows[event.symbol].update(event.price, event.quantity, event.isBuyerMaker);
}

}  // namespace aggregation
