#include "app/snapshot_output_adapter.hpp"

#include <gtest/gtest.h>

#include <aggregation/symbol_snapshot.hpp>
#include <output/i_snapshot_printer.hpp>
#include <string>
#include <vector>

namespace app::test {

TEST(SnapshotOutputAdapterTest, test_adaptAggregationSnapshotsToOutput_emptyInput) {
    std::vector<aggregation::SymbolSnapshot> input;
    auto result = adaptAggregationSnapshotsToOutput(input);
    EXPECT_TRUE(result.empty());
}

TEST(SnapshotOutputAdapterTest, test_adaptAggregationSnapshotsToOutput_singleSnapshot) {
    std::vector<aggregation::SymbolSnapshot> input = {
        {.symbol = "BTCUSDT",
         .tradeCount = 10,
         .totalVolume = 500000.0,
         .minPrice = 49000.0,
         .maxPrice = 51000.0,
         .buyCount = 6,
         .sellCount = 4},
    };

    auto result = adaptAggregationSnapshotsToOutput(input);

    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0].symbol, "BTCUSDT");
    EXPECT_EQ(result[0].tradeCount, 10U);
    EXPECT_DOUBLE_EQ(result[0].totalVolume, 500000.0);
    EXPECT_DOUBLE_EQ(result[0].minPrice, 49000.0);
    EXPECT_DOUBLE_EQ(result[0].maxPrice, 51000.0);
    EXPECT_EQ(result[0].buyCount, 6U);
    EXPECT_EQ(result[0].sellCount, 4U);
}

TEST(SnapshotOutputAdapterTest,
     test_adaptAggregationSnapshotsToOutput_multipleSnapshotsPreserveOrder) {
    std::vector<aggregation::SymbolSnapshot> input = {
        {.symbol = "BTCUSDT", .tradeCount = 1},
        {.symbol = "ETHUSDT", .tradeCount = 2},
        {.symbol = "SOLUSDT", .tradeCount = 3},
    };

    auto result = adaptAggregationSnapshotsToOutput(input);

    ASSERT_EQ(result.size(), 3U);
    EXPECT_EQ(result[0].symbol, "BTCUSDT");
    EXPECT_EQ(result[1].symbol, "ETHUSDT");
    EXPECT_EQ(result[2].symbol, "SOLUSDT");
    EXPECT_EQ(result[0].tradeCount, 1U);
    EXPECT_EQ(result[1].tradeCount, 2U);
    EXPECT_EQ(result[2].tradeCount, 3U);
}

TEST(SnapshotOutputAdapterTest, test_adaptAggregationSnapshotsToOutput_zeroFieldsConverted) {
    std::vector<aggregation::SymbolSnapshot> input = {
        {.symbol = "XRPUSDT"},
    };

    auto result = adaptAggregationSnapshotsToOutput(input);

    ASSERT_EQ(result.size(), 1U);
    EXPECT_EQ(result[0].tradeCount, 0U);
    EXPECT_DOUBLE_EQ(result[0].totalVolume, 0.0);
    EXPECT_DOUBLE_EQ(result[0].minPrice, 0.0);
    EXPECT_DOUBLE_EQ(result[0].maxPrice, 0.0);
    EXPECT_EQ(result[0].buyCount, 0U);
    EXPECT_EQ(result[0].sellCount, 0U);
}

}  // namespace app::test
