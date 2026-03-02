#include <gtest/gtest.h>

#include <array>
#include <market/market_streams_controller.hpp>
#include <memory>
#include <stdexcept>

namespace market::test {

class MarketStreamsControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_controller = std::make_unique<MarketStreamsController>();
    }

    void TearDown() override {
        if (m_controller) {
            m_controller->stopAll();
        }
    }

    std::unique_ptr<MarketStreamsController> m_controller;
};

TEST_F(MarketStreamsControllerTest, test_hasHandlers_initiallyFalse) {
    EXPECT_FALSE(m_controller->hasHandlers());
}

TEST_F(MarketStreamsControllerTest, test_addHandler_returnsNonNull) {
    const std::array pairs = {CurrencyPair::BTCUSDT};

    auto handler = m_controller->addHandler(Exchange::Binance, pairs);
    EXPECT_NE(handler, nullptr);
}

TEST_F(MarketStreamsControllerTest, test_addHandler_hasHandlersAfterAdd) {
    const std::array pairs = {CurrencyPair::BTCUSDT};

    m_controller->addHandler(Exchange::Binance, pairs);
    EXPECT_TRUE(m_controller->hasHandlers());
}

TEST_F(MarketStreamsControllerTest, test_addHandler_multipleHandlers) {
    const std::array pairs1 = {CurrencyPair::BTCUSDT};
    const std::array pairs2 = {CurrencyPair::ETHUSDT, CurrencyPair::SOLUSDT};

    auto h1 = m_controller->addHandler(Exchange::Binance, pairs1);
    auto h2 = m_controller->addHandler(Exchange::Binance, pairs2);

    EXPECT_NE(h1, nullptr);
    EXPECT_NE(h2, nullptr);
    EXPECT_NE(h1, h2);
    EXPECT_TRUE(m_controller->hasHandlers());
}

TEST_F(MarketStreamsControllerTest, test_addHandler_unsupportedExchangeThrows) {
    const std::array pairs = {CurrencyPair::BTCUSDT};

    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    auto badExchange = static_cast<Exchange>(255);
    EXPECT_THROW(m_controller->addHandler(badExchange, pairs), std::invalid_argument);
}

TEST_F(MarketStreamsControllerTest, test_startAll_emptyDoesNotThrow) {
    EXPECT_NO_THROW(m_controller->startAll());
}

TEST_F(MarketStreamsControllerTest, test_stopAll_emptyDoesNotThrow) {
    EXPECT_NO_THROW(m_controller->stopAll());
}

TEST_F(MarketStreamsControllerTest, test_addHandler_supportsTradeSubscription) {
    const std::array pairs = {CurrencyPair::BTCUSDT};

    auto handler = m_controller->addHandler(Exchange::Binance, pairs);
    bool subscribed = false;
    EXPECT_NO_THROW(handler->subscribeOnTrade([&](const TradeEvent&) { subscribed = true; }));
}

TEST_F(MarketStreamsControllerTest, test_moveConstructor_transfersOwnership) {
    const std::array pairs = {CurrencyPair::BTCUSDT};
    m_controller->addHandler(Exchange::Binance, pairs);

    MarketStreamsController moved(std::move(*m_controller));
    m_controller.reset();  // prevent TearDown from using moved-from object
    EXPECT_TRUE(moved.hasHandlers());
}

}  // namespace market::test
