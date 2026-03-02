#include "application.hpp"

#include <poll.h>
#include <pthread.h>
#include <spdlog/spdlog.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include <aggregation/src/statistics_aggregator.hpp>
#include <aggregation/symbol_snapshot.hpp>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <market/trade_event.hpp>
#include <memory>
#include <output/snapshot_printer_factory.hpp>
#include <vector>

#include "snapshot_output_adapter.hpp"

namespace app {

Application::Application(const ApplicationConfig& config) {
    // Create aggregator
    m_aggregator = std::make_shared<aggregation::StatisticsAggregator>(
        std::chrono::seconds(config.windowLength));

    // Create market stream handlers and subscribe them to aggregator
    for (const auto& exchangeConfig : config.exchanges) {
        if (exchangeConfig.streams.empty()) {
            spdlog::warn("Skipping exchange with empty streams list");
            continue;
        }

        auto handler = m_marketStreams.addHandler(exchangeConfig.exchange, exchangeConfig.streams);
        handler->subscribeOnTrade(
            [agg = m_aggregator](const market::TradeEvent& event) { agg->addTrade(event); });
    }

    // Create output
    m_output = output::SnapshotPrinterFactory::createFileOutput(output::OutputFormat::PlainTextV1,
                                                                config.outputPath);

    m_aggregator->subscribeOnWindowElapsed(
        [this](const std::vector<aggregation::SymbolSnapshot>& snapshots) {
            if (snapshots.empty()) {
                return;
            }

            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 std::chrono::system_clock::now().time_since_epoch())
                                 .count();

            const auto outputSnapshots = adaptAggregationSnapshotsToOutput(snapshots);
            m_output->write(outputSnapshots, timestamp);

            spdlog::debug("Flushed {} symbols", snapshots.size());
        });
}

Application::~Application() {
    stop();
}

void Application::run() {
    if (m_running) {
        return;
    }

    if (!m_marketStreams.hasHandlers()) {
        spdlog::error("No active market handlers. Application will not start.");
        return;
    }

    m_running = true;

    spdlog::info("Starting application...");

    if (!setupSignalHandling()) {
        m_running = false;
        return;
    }

    m_marketStreams.startAll();

    m_aggregator->start();

    pollfd signalPollFd{};
    signalPollFd.fd = m_signalFd;
    signalPollFd.events = POLLIN;

    while (m_running) {
        const int pollResult = ::poll(&signalPollFd, 1, 200);
        if (pollResult < 0) {
            if (errno == EINTR) {
                continue;
            }
            spdlog::error("Signal poll failed: {}", errno);
            stop();
            break;
        }

        if (pollResult == 0) {
            continue;
        }

        if ((signalPollFd.revents & POLLIN) != 0) {
            signalfd_siginfo signalInfo{};
            const auto bytesRead = ::read(m_signalFd, &signalInfo, sizeof(signalfd_siginfo));
            if (bytesRead == sizeof(signalfd_siginfo)) {
                spdlog::info("Received signal {}, shutting down...", signalInfo.ssi_signo);
            }

            stop();
            break;
        }
    }

    if (m_signalFd >= 0) {
        ::close(m_signalFd);
        m_signalFd = -1;
    }

    spdlog::info("Application stopped.");
}

void Application::stop() {
    if (!m_running) {
        return;
    }
    m_running = false;

    m_aggregator->stop();

    m_marketStreams.stopAll();

    spdlog::info("Stopping application...");
}

bool Application::setupSignalHandling() {
    sigset_t signalMask;
    sigemptyset(&signalMask);
    sigaddset(&signalMask, SIGINT);
    sigaddset(&signalMask, SIGTERM);

    if (::pthread_sigmask(SIG_BLOCK, &signalMask, nullptr) != 0) {
        spdlog::error("Failed to block termination signals: {}", errno);
        return false;
    }

    m_signalFd = ::signalfd(-1, &signalMask, SFD_CLOEXEC);
    if (m_signalFd < 0) {
        spdlog::error("Failed to create signalfd: {}", errno);
        return false;
    }

    return true;
}

}  // namespace app
