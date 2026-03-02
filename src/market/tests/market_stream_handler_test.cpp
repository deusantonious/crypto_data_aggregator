#include "market_stream_handler.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>

#include "mocks/mock_exchange_connection_manager.hpp"

namespace market::test {

using ::testing::_;
using ::testing::Invoke;
using ::testing::StrictMock;

class MarketStreamHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_mockConnMgr = std::make_shared<StrictMock<MockExchangeConnectionManager>>();
    }

    /// A parse strategy that always succeeds with deterministic values.
    static std::optional<TradeEvent> validParseStrategy(const std::string& /*raw*/) {
        return TradeEvent{
            .symbol = "BTCUSDT",
            .price = 50000.0,
            .quantity = 1.5,
            .isBuyerMaker = true,
            .tradeTimeMs = 1700000000000,
        };
    }

    /// A parse strategy that always returns nullopt (e.g. non-trade message).
    static std::optional<TradeEvent> nullParseStrategy(const std::string& /*raw*/) {
        return std::nullopt;
    }

    std::shared_ptr<StrictMock<MockExchangeConnectionManager>> m_mockConnMgr;
};

TEST_F(MarketStreamHandlerTest, test_start_setsCallbackAndConnects) {
    MarketStreamHandler handler(m_mockConnMgr, validParseStrategy);

    EXPECT_CALL(*m_mockConnMgr, setMessageCallback(_)).Times(1);
    EXPECT_CALL(*m_mockConnMgr, connect()).Times(1);

    handler.start();
}

TEST_F(MarketStreamHandlerTest, test_stop_clearsCallbackAndDisconnects) {
    MarketStreamHandler handler(m_mockConnMgr, validParseStrategy);

    EXPECT_CALL(*m_mockConnMgr, setMessageCallback(_)).Times(1);
    EXPECT_CALL(*m_mockConnMgr, disconnect()).Times(1);

    handler.stop();
}

TEST_F(MarketStreamHandlerTest, test_start_incomingMessageDispatchedToTradeCallback) {
    IExchangeConnectionManager::MessageCallback capturedCb;

    EXPECT_CALL(*m_mockConnMgr, setMessageCallback(_))
        .WillOnce(Invoke(
            [&](IExchangeConnectionManager::MessageCallback cb) { capturedCb = std::move(cb); }));
    EXPECT_CALL(*m_mockConnMgr, connect());

    MarketStreamHandler handler(m_mockConnMgr, validParseStrategy);
    TradeEvent received{};
    handler.subscribeOnTrade([&](const TradeEvent& e) { received = e; });
    handler.start();

    ASSERT_TRUE(capturedCb);
    capturedCb(R"({"dummy":"json"})");

    EXPECT_EQ(received.symbol, "BTCUSDT");
    EXPECT_DOUBLE_EQ(received.price, 50000.0);
    EXPECT_DOUBLE_EQ(received.quantity, 1.5);
    EXPECT_TRUE(received.isBuyerMaker);
    EXPECT_EQ(received.tradeTimeMs, 1700000000000);
}

TEST_F(MarketStreamHandlerTest, test_start_unparsableMessageDoesNotInvokeCallback) {
    IExchangeConnectionManager::MessageCallback capturedCb;

    EXPECT_CALL(*m_mockConnMgr, setMessageCallback(_))
        .WillOnce(Invoke(
            [&](IExchangeConnectionManager::MessageCallback cb) { capturedCb = std::move(cb); }));
    EXPECT_CALL(*m_mockConnMgr, connect());

    MarketStreamHandler handler(m_mockConnMgr, nullParseStrategy);
    bool callbackInvoked = false;
    handler.subscribeOnTrade([&](const TradeEvent&) { callbackInvoked = true; });
    handler.start();

    ASSERT_TRUE(capturedCb);
    capturedCb("garbage");

    EXPECT_FALSE(callbackInvoked);
}

TEST_F(MarketStreamHandlerTest, test_start_messageBeforeSubscribeDoesNotCrash) {
    IExchangeConnectionManager::MessageCallback capturedCb;

    EXPECT_CALL(*m_mockConnMgr, setMessageCallback(_))
        .WillOnce(Invoke(
            [&](IExchangeConnectionManager::MessageCallback cb) { capturedCb = std::move(cb); }));
    EXPECT_CALL(*m_mockConnMgr, connect());

    MarketStreamHandler handler(m_mockConnMgr, validParseStrategy);

    handler.start();

    ASSERT_TRUE(capturedCb);
    EXPECT_NO_THROW(capturedCb(R"({"dummy":"json"})"));
}

}  // namespace market::test
