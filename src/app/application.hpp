#pragma once

#include <aggregation/i_statistics_aggregator.hpp>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <market/market_streams_controller.hpp>
#include <memory>
#include <output/i_snapshot_printer.hpp>

namespace app {

/// @brief Configuration for a single exchange: which exchange and which currency pairs to stream.
struct ExchangeStreamsConfig {
    market::Exchange exchange{market::Exchange::Binance};
    std::vector<market::CurrencyPair> streams;
};

/// @brief Top-level application configuration loaded from the JSON config file.
struct ApplicationConfig {
    std::vector<ExchangeStreamsConfig> exchanges;
    int windowLength{};                ///< Aggregation window length in seconds.
    std::filesystem::path outputPath;  ///< Filesystem path for the output file.
};

/// @brief Top-level application class that wires together market streams, aggregation,
///        and output subsystems, and runs the main event loop.
class Application {
public:
    /// @brief Constructs the application: creates the aggregator, market stream handlers,
    ///        and output printer based on the provided configuration.
    /// @param config Application configuration specifying exchanges, window length, and output
    /// path.
    explicit Application(const ApplicationConfig& config);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    /// @brief Starts the application: connects to exchanges, begins aggregation, and blocks
    ///        until a SIGINT/SIGTERM signal is received.
    void run();

    /// @brief Gracefully stops all subsystems (aggregator, market streams). Safe to call
    ///        multiple times; subsequent calls are no-ops.
    void stop();

private:
    /// @brief Blocks SIGINT/SIGTERM on all threads and creates a signalfd for poll-based
    ///        signal handling in the main loop.
    /// @return True on success, false if signal masking or signalfd creation fails.
    bool setupSignalHandling();

    market::MarketStreamsController m_marketStreams;
    std::shared_ptr<aggregation::IStatisticsAggregator> m_aggregator;
    std::unique_ptr<output::ISnapshotPrinter> m_output;

    std::atomic<bool> m_running{false};
    int m_signalFd{-1};
};

}  // namespace app
