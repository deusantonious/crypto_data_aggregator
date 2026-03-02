#include "plain_text_formatter.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <output/i_snapshot_printer.hpp>
#include <string>
#include <vector>

namespace output::test {

namespace {

SymbolSnapshot makeSnapshot(const std::string& symbol, uint64_t trades, double volume, double min,
                            double max, uint64_t buys, uint64_t sells) {
    return {
        .symbol = symbol,
        .tradeCount = trades,
        .totalVolume = volume,
        .minPrice = min,
        .maxPrice = max,
        .buyCount = buys,
        .sellCount = sells,
    };
}

}  // namespace

TEST(PlainTextFormatterTest, test_format_singleSymbolFormatted) {
    PlainTextFormatter formatter;
    std::vector<SymbolSnapshot> snapshots = {
        makeSnapshot("BTCUSDT", 10, 500000.0, 49000.0, 51000.0, 6, 4)};

    // 1700000000000 ms = 2023-11-14T22:13:20Z
    auto result = formatter.format(snapshots, 1700000000000);

    EXPECT_NE(result.find("timestamp=2023-11-14T22:13:20Z"), std::string::npos);
    EXPECT_NE(result.find("symbol=BTCUSDT"), std::string::npos);
    EXPECT_NE(result.find("trades=10"), std::string::npos);
    EXPECT_NE(result.find("volume=500000.0"), std::string::npos);
    EXPECT_NE(result.find("min=49000.0"), std::string::npos);
    EXPECT_NE(result.find("max=51000.0"), std::string::npos);
    EXPECT_NE(result.find("buy=6"), std::string::npos);
    EXPECT_NE(result.find("sell=4"), std::string::npos);
}

TEST(PlainTextFormatterTest, test_format_multipleSymbols) {
    PlainTextFormatter formatter;
    std::vector<SymbolSnapshot> snapshots = {
        makeSnapshot("BTCUSDT", 5, 100000.0, 49000.0, 51000.0, 3, 2),
        makeSnapshot("ETHUSDT", 3, 9600.0, 3100.0, 3300.0, 1, 2),
    };

    auto result = formatter.format(snapshots, 1700000000000);

    EXPECT_NE(result.find("symbol=BTCUSDT"), std::string::npos);
    EXPECT_NE(result.find("symbol=ETHUSDT"), std::string::npos);
}

TEST(PlainTextFormatterTest, test_format_emptySnapshotsStillHasTimestamp) {
    PlainTextFormatter formatter;
    std::vector<SymbolSnapshot> empty;

    auto result = formatter.format(empty, 1700000000000);

    EXPECT_NE(result.find("timestamp="), std::string::npos);
    EXPECT_EQ(result.find("symbol="), std::string::npos);
}

TEST(PlainTextFormatterTest, test_format_endsWithDoubleNewline) {
    PlainTextFormatter formatter;
    std::vector<SymbolSnapshot> snapshots = {
        makeSnapshot("BTCUSDT", 1, 50000.0, 50000.0, 50000.0, 1, 0)};

    auto result = formatter.format(snapshots, 1700000000000);

    // Format ends with "\n\n" (line for symbol + trailing blank line).
    ASSERT_GE(result.size(), 2U);
    EXPECT_EQ(result.substr(result.size() - 2), "\n\n");
}

TEST(PlainTextFormatterTest, test_format_wholeNumbersGetDecimalPoint) {
    PlainTextFormatter formatter;
    std::vector<SymbolSnapshot> snapshots = {
        makeSnapshot("BTCUSDT", 1, 1000.0, 500.0, 500.0, 1, 0)};

    auto result = formatter.format(snapshots, 1700000000000);

    // Whole numbers should have ".0" appended.
    EXPECT_NE(result.find("volume=1000.0"), std::string::npos);
    EXPECT_NE(result.find("min=500.0"), std::string::npos);
}

TEST(PlainTextFormatterTest, test_format_fractionalValuesPreserved) {
    PlainTextFormatter formatter;
    std::vector<SymbolSnapshot> snapshots = {
        makeSnapshot("BTCUSDT", 1, 12345.6789, 100.25, 200.75, 0, 1)};

    auto result = formatter.format(snapshots, 1700000000000);

    EXPECT_NE(result.find("volume=12345.6789"), std::string::npos);
    EXPECT_NE(result.find("min=100.25"), std::string::npos);
    EXPECT_NE(result.find("max=200.75"), std::string::npos);
}

}  // namespace output::test
