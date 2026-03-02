#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <market/i_market_stream_handler.hpp>
#include <market/market_streams_controller.hpp>
#include <memory>
#include <span>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "binance/binance_config.hpp"
#include "binance/binance_connection_manager.hpp"
#include "binance/binance_trade_message_parser.hpp"
#include "market_stream_handler.hpp"

namespace market {

class MarketStreamsController::Impl {
public:
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;

    Impl()
        : workGuard(boost::asio::make_work_guard(ioContext)),
          worker([this]() { ioContext.run(); }) {}

    ~Impl() {
        stopAll();
        workGuard.reset();
        ioContext.stop();
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::shared_ptr<IMarketStreamHandler> addHandler(Exchange exchange,
                                                     std::span<const CurrencyPair> pairs) {
        switch (exchange) {
            case Exchange::Binance: {
                auto connectionMgr = std::make_shared<BinanceConnectionManager>(
                    ioContext, std::string{binance::kWsHost}, std::string{binance::kWsPort},
                    binance::buildStreamPath(pairs));
                std::shared_ptr<IMarketStreamHandler> handler =
                    std::make_shared<MarketStreamHandler>(std::move(connectionMgr),
                                                          binance::parseBinanceTradeMessage);
                handlers.push_back(handler);
                return handler;
            }
            default:
                throw std::invalid_argument("Unsupported exchange");
        }
    }

    void startAll() {
        for (auto& handler : handlers) {
            handler->start();
        }
    }

    void stopAll() {
        for (auto& handler : handlers) {
            handler->stop();
        }
    }

    [[nodiscard]] bool hasHandlers() const {
        return !handlers.empty();
    }

private:
    boost::asio::io_context ioContext;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard;
    std::thread worker;
    std::vector<std::shared_ptr<IMarketStreamHandler>> handlers;
};

MarketStreamsController::MarketStreamsController() : m_impl(std::make_unique<Impl>()) {}

MarketStreamsController::~MarketStreamsController() = default;

MarketStreamsController::MarketStreamsController(MarketStreamsController&&) noexcept = default;
MarketStreamsController& MarketStreamsController::operator=(MarketStreamsController&&) noexcept =
    default;

std::shared_ptr<IMarketStreamHandler> MarketStreamsController::addHandler(
    Exchange exchange, std::span<const CurrencyPair> pairs) {
    return m_impl->addHandler(exchange, pairs);
}

void MarketStreamsController::startAll() {
    m_impl->startAll();
}

void MarketStreamsController::stopAll() {
    m_impl->stopAll();
}

bool MarketStreamsController::hasHandlers() const {
    return m_impl->hasHandlers();
}

}  // namespace market
