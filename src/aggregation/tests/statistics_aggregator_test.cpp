#include "statistics_aggregator.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <market/trade_event.hpp>
#include <mutex>
#include <vector>

namespace aggregation::test {

using namespace std::chrono_literals;

namespace {

market::TradeEvent makeTrade(const std::string& symbol, double price, double quantity,
                             bool isBuyerMaker = false) {
    return {
        .symbol = symbol,
        .price = price,
        .quantity = quantity,
        .isBuyerMaker = isBuyerMaker,
        .tradeTimeMs = 0,
    };
}

}  // namespace

class StatisticsAggregatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_agg = std::make_unique<StatisticsAggregator>(1s);
    }

    void TearDown() override {
        if (m_agg) {
            m_agg->stop();
        }
    }

    std::unique_ptr<StatisticsAggregator> m_agg;
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

TEST_F(StatisticsAggregatorTest, test_constructor_validDuration) {
    EXPECT_NO_THROW(StatisticsAggregator agg(1s));
}

TEST_F(StatisticsAggregatorTest, test_constructor_zeroDurationDoesNotThrow) {
    EXPECT_NO_THROW(StatisticsAggregator agg(0s));
}

TEST_F(StatisticsAggregatorTest, test_constructor_negativeDurationDoesNotThrow) {
    EXPECT_NO_THROW(StatisticsAggregator agg(-1s));
}

TEST_F(StatisticsAggregatorTest, test_addTrade_beforeStartDoesNotThrow) {
    EXPECT_NO_THROW(m_agg->addTrade(makeTrade("BTCUSDT", 50000.0, 1.0)));
}

TEST_F(StatisticsAggregatorTest, test_start_andStopWithoutCrash) {
    m_agg->start();
    m_agg->stop();
}

TEST_F(StatisticsAggregatorTest, test_start_doubleStartDoesNotThrow) {
    m_agg->start();
    EXPECT_NO_THROW(m_agg->start());
}

TEST_F(StatisticsAggregatorTest, test_stop_withoutStartDoesNotThrow) {
    EXPECT_NO_THROW(m_agg->stop());
}

TEST_F(StatisticsAggregatorTest, test_subscribeOnWindowElapsed_callbackFiredAfterWindowElapses) {
    std::vector<SymbolSnapshot> received;

    m_agg->subscribeOnWindowElapsed([&](const std::vector<SymbolSnapshot>& snapshots) {
        std::lock_guard lock(m_mtx);
        received = snapshots;
        m_cv.notify_all();
    });

    m_agg->addTrade(makeTrade("BTCUSDT", 50000.0, 1.0, false));
    m_agg->addTrade(makeTrade("BTCUSDT", 51000.0, 2.0, true));
    m_agg->start();

    {
        std::unique_lock lock(m_mtx);
        ASSERT_TRUE(m_cv.wait_for(lock, 3s, [&] { return !received.empty(); }));
    }

    ASSERT_EQ(received.size(), 1U);
    EXPECT_EQ(received[0].symbol, "BTCUSDT");
    EXPECT_EQ(received[0].tradeCount, 2U);
    EXPECT_DOUBLE_EQ(received[0].totalVolume, 50000.0 * 1.0 + 51000.0 * 2.0);
    EXPECT_DOUBLE_EQ(received[0].minPrice, 50000.0);
    EXPECT_DOUBLE_EQ(received[0].maxPrice, 51000.0);
    EXPECT_EQ(received[0].buyCount, 1U);
    EXPECT_EQ(received[0].sellCount, 1U);
}

TEST_F(StatisticsAggregatorTest, test_subscribeOnWindowElapsed_multipleSymbolsReportedSeparately) {
    std::vector<SymbolSnapshot> received;

    m_agg->subscribeOnWindowElapsed([&](const std::vector<SymbolSnapshot>& snapshots) {
        std::lock_guard lock(m_mtx);
        received = snapshots;
        m_cv.notify_all();
    });

    m_agg->addTrade(makeTrade("BTCUSDT", 50000.0, 1.0));
    m_agg->addTrade(makeTrade("ETHUSDT", 3200.0, 5.0));
    m_agg->start();

    {
        std::unique_lock lock(m_mtx);
        ASSERT_TRUE(m_cv.wait_for(lock, 3s, [&] { return !received.empty(); }));
    }

    ASSERT_EQ(received.size(), 2U);

    // Sort by symbol for deterministic assertions (unordered_map iteration order).
    std::sort(received.begin(), received.end(),
              [](const SymbolSnapshot& a, const SymbolSnapshot& b) { return a.symbol < b.symbol; });

    EXPECT_EQ(received[0].symbol, "BTCUSDT");
    EXPECT_EQ(received[0].tradeCount, 1U);

    EXPECT_EQ(received[1].symbol, "ETHUSDT");
    EXPECT_EQ(received[1].tradeCount, 1U);
}

TEST_F(StatisticsAggregatorTest, test_subscribeOnWindowElapsed_windowResetsAfterCallback) {
    int callbackCount = 0;
    std::vector<SymbolSnapshot> firstBatch;
    std::vector<SymbolSnapshot> secondBatch;

    m_agg->subscribeOnWindowElapsed([&](const std::vector<SymbolSnapshot>& snapshots) {
        std::lock_guard lock(m_mtx);
        callbackCount++;
        if (callbackCount == 1) {
            firstBatch = snapshots;
        } else if (callbackCount == 2) {
            secondBatch = snapshots;
        }
        m_cv.notify_all();
    });

    m_agg->addTrade(makeTrade("BTCUSDT", 50000.0, 1.0));
    m_agg->start();

    {
        std::unique_lock lock(m_mtx);
        ASSERT_TRUE(m_cv.wait_for(lock, 5s, [&] { return callbackCount >= 2; }));
    }

    // First window captured the trade.
    ASSERT_EQ(firstBatch.size(), 1U);
    EXPECT_EQ(firstBatch[0].tradeCount, 1U);

    // Second window: symbol key persists but counters are reset to zero.
    ASSERT_EQ(secondBatch.size(), 1U);
    EXPECT_EQ(secondBatch[0].tradeCount, 0U);
    EXPECT_DOUBLE_EQ(secondBatch[0].totalVolume, 0.0);
}

TEST_F(StatisticsAggregatorTest, test_subscribeOnWindowElapsed_noCallbackWhenNoTrades) {
    bool called = false;

    m_agg->subscribeOnWindowElapsed([&](const std::vector<SymbolSnapshot>&) {
        std::lock_guard lock(m_mtx);
        called = true;
        m_cv.notify_all();
    });

    m_agg->start();

    {
        std::unique_lock lock(m_mtx);
        // Wait slightly longer than one window — callback should NOT fire.
        m_cv.wait_for(lock, 2s, [&] { return called; });
    }

    EXPECT_FALSE(called);
}

TEST_F(StatisticsAggregatorTest, test_subscribeOnWindowElapsed_multipleSubscribersAllNotified) {
    int notifiedCount = 0;

    auto subscriber = [&](const std::vector<SymbolSnapshot>&) {
        std::lock_guard lock(m_mtx);
        notifiedCount++;
        m_cv.notify_all();
    };

    m_agg->subscribeOnWindowElapsed(subscriber);
    m_agg->subscribeOnWindowElapsed(subscriber);

    m_agg->addTrade(makeTrade("BTCUSDT", 50000.0, 1.0));
    m_agg->start();

    {
        std::unique_lock lock(m_mtx);
        ASSERT_TRUE(m_cv.wait_for(lock, 3s, [&] { return notifiedCount >= 2; }));
    }

    EXPECT_EQ(notifiedCount, 2);
}

}  // namespace aggregation::test
