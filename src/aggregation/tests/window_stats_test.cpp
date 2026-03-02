#include "window_stats.hpp"

#include <gtest/gtest.h>

#include <aggregation/symbol_snapshot.hpp>
#include <limits>

namespace aggregation::test {

TEST(WindowStatsTest, test_constructor_initialValues) {
    WindowStats stats;

    EXPECT_EQ(stats.tradeCount(), 0U);
    EXPECT_DOUBLE_EQ(stats.totalVolume(), 0.0);
    EXPECT_EQ(stats.buyCount(), 0U);
    EXPECT_EQ(stats.sellCount(), 0U);
}

TEST(WindowStatsTest, test_constructor_initialMinMaxAreExtreme) {
    WindowStats stats;

    EXPECT_DOUBLE_EQ(stats.minPrice(), std::numeric_limits<double>::max());
    EXPECT_DOUBLE_EQ(stats.maxPrice(), std::numeric_limits<double>::lowest());
}

TEST(WindowStatsTest, test_update_singleBuyTrade) {
    WindowStats stats;
    stats.update(50000.0, 1.5, /*isBuyerMaker=*/false);

    EXPECT_EQ(stats.tradeCount(), 1U);
    EXPECT_DOUBLE_EQ(stats.totalVolume(), 50000.0 * 1.5);
    EXPECT_DOUBLE_EQ(stats.minPrice(), 50000.0);
    EXPECT_DOUBLE_EQ(stats.maxPrice(), 50000.0);
    EXPECT_EQ(stats.buyCount(), 1U);
    EXPECT_EQ(stats.sellCount(), 0U);
}

TEST(WindowStatsTest, test_update_singleSellTrade) {
    WindowStats stats;
    stats.update(3200.0, 10.0, /*isBuyerMaker=*/true);

    EXPECT_EQ(stats.tradeCount(), 1U);
    EXPECT_DOUBLE_EQ(stats.totalVolume(), 3200.0 * 10.0);
    EXPECT_EQ(stats.buyCount(), 0U);
    EXPECT_EQ(stats.sellCount(), 1U);
}

TEST(WindowStatsTest, test_update_multipleTradesAggregatesCorrectly) {
    WindowStats stats;
    stats.update(100.0, 2.0, false);  // buy,   volume = 200
    stats.update(110.0, 3.0, true);   // sell,  volume = 330
    stats.update(90.0, 1.0, false);   // buy,   volume = 90

    EXPECT_EQ(stats.tradeCount(), 3U);
    EXPECT_DOUBLE_EQ(stats.totalVolume(), 200.0 + 330.0 + 90.0);
    EXPECT_DOUBLE_EQ(stats.minPrice(), 90.0);
    EXPECT_DOUBLE_EQ(stats.maxPrice(), 110.0);
    EXPECT_EQ(stats.buyCount(), 2U);
    EXPECT_EQ(stats.sellCount(), 1U);
}

TEST(WindowStatsTest, test_reset_clearsAllFields) {
    WindowStats stats;
    stats.update(100.0, 1.0, false);
    stats.update(200.0, 2.0, true);
    stats.reset();

    EXPECT_EQ(stats.tradeCount(), 0U);
    EXPECT_DOUBLE_EQ(stats.totalVolume(), 0.0);
    EXPECT_DOUBLE_EQ(stats.minPrice(), std::numeric_limits<double>::max());
    EXPECT_DOUBLE_EQ(stats.maxPrice(), std::numeric_limits<double>::lowest());
    EXPECT_EQ(stats.buyCount(), 0U);
    EXPECT_EQ(stats.sellCount(), 0U);
}

TEST(WindowStatsTest, test_reset_allowsReaccumulation) {
    WindowStats stats;
    stats.update(100.0, 1.0, false);
    stats.reset();
    stats.update(50.0, 4.0, true);

    EXPECT_EQ(stats.tradeCount(), 1U);
    EXPECT_DOUBLE_EQ(stats.totalVolume(), 50.0 * 4.0);
    EXPECT_DOUBLE_EQ(stats.minPrice(), 50.0);
    EXPECT_DOUBLE_EQ(stats.maxPrice(), 50.0);
    EXPECT_EQ(stats.buyCount(), 0U);
    EXPECT_EQ(stats.sellCount(), 1U);
}

TEST(WindowStatsTest, test_toSnapshot_capturesSymbolAndValues) {
    WindowStats stats;
    stats.update(100.0, 2.0, false);
    stats.update(200.0, 3.0, true);

    auto snap = stats.toSnapshot("BTCUSDT");

    EXPECT_EQ(snap.symbol, "BTCUSDT");
    EXPECT_EQ(snap.tradeCount, 2U);
    EXPECT_DOUBLE_EQ(snap.totalVolume, 100.0 * 2.0 + 200.0 * 3.0);
    EXPECT_DOUBLE_EQ(snap.minPrice, 100.0);
    EXPECT_DOUBLE_EQ(snap.maxPrice, 200.0);
    EXPECT_EQ(snap.buyCount, 1U);
    EXPECT_EQ(snap.sellCount, 1U);
}

TEST(WindowStatsTest, test_toSnapshot_freshStatsReturnsDefaults) {
    WindowStats stats;
    auto snap = stats.toSnapshot("ETHUSDT");

    EXPECT_EQ(snap.symbol, "ETHUSDT");
    EXPECT_EQ(snap.tradeCount, 0U);
    EXPECT_DOUBLE_EQ(snap.totalVolume, 0.0);
    EXPECT_DOUBLE_EQ(snap.minPrice, std::numeric_limits<double>::max());
    EXPECT_DOUBLE_EQ(snap.maxPrice, std::numeric_limits<double>::lowest());
}

}  // namespace aggregation::test
