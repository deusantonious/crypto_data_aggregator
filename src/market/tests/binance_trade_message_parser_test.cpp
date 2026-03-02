#include "binance/binance_trade_message_parser.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>

namespace binance::test {

namespace {

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
std::string makeValidTradeJson(const std::string& symbol = "BTCUSDT",
                               const std::string& price = "50000.00",
                               const std::string& quantity = "1.50000000", bool isBuyerMaker = true,
                               int64_t tradeTimeMs = 1700000000000) {
    return R"({"e":"trade","E":1700000000001,"s":")" + symbol + R"(","t":123456,"p":")" + price +
           R"(","q":")" + quantity + R"(","T":)" + std::to_string(tradeTimeMs) + R"(,"m":)" +
           (isBuyerMaker ? "true" : "false") + R"(,"M":true})";
}

}  // namespace

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_validTradeMessage) {
    auto result = parseBinanceTradeMessage(makeValidTradeJson());

    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    const auto& event = result.value();
    EXPECT_EQ(event.symbol, "BTCUSDT");
    EXPECT_DOUBLE_EQ(event.price, 50000.00);
    EXPECT_DOUBLE_EQ(event.quantity, 1.5);
    EXPECT_TRUE(event.isBuyerMaker);
    EXPECT_EQ(event.tradeTimeMs, 1700000000000);
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_sellerMakerParsed) {
    auto result = parseBinanceTradeMessage(
        makeValidTradeJson("ETHUSDT", "3200.50", "10.0", false, 1700000099999));

    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    const auto& event = result.value();
    EXPECT_EQ(event.symbol, "ETHUSDT");
    EXPECT_DOUBLE_EQ(event.price, 3200.50);
    EXPECT_DOUBLE_EQ(event.quantity, 10.0);
    EXPECT_FALSE(event.isBuyerMaker);
    EXPECT_EQ(event.tradeTimeMs, 1700000099999);
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_verySmallPriceAndQuantity) {
    auto result = parseBinanceTradeMessage(makeValidTradeJson("SOLUSDT", "0.00000001", "0.001"));

    ASSERT_TRUE(result.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    const auto& event = result.value();
    EXPECT_DOUBLE_EQ(event.price, 0.00000001);
    EXPECT_DOUBLE_EQ(event.quantity, 0.001);
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_nonTradeEventReturnsNullopt) {
    const std::string json = R"({"e":"aggTrade","s":"BTCUSDT","p":"50000","q":"1","T":0,"m":true})";
    EXPECT_FALSE(parseBinanceTradeMessage(json).has_value());
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_missingEventFieldReturnsNullopt) {
    const std::string json = R"({"s":"BTCUSDT","p":"50000","q":"1","T":0,"m":true})";
    EXPECT_FALSE(parseBinanceTradeMessage(json).has_value());
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_missingSymbolReturnsNullopt) {
    const std::string json = R"({"e":"trade","p":"50000","q":"1","T":0,"m":true})";
    EXPECT_FALSE(parseBinanceTradeMessage(json).has_value());
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_missingPriceReturnsNullopt) {
    const std::string json = R"({"e":"trade","s":"BTCUSDT","q":"1","T":0,"m":true})";
    EXPECT_FALSE(parseBinanceTradeMessage(json).has_value());
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_missingQuantityReturnsNullopt) {
    const std::string json = R"({"e":"trade","s":"BTCUSDT","p":"50000","T":0,"m":true})";
    EXPECT_FALSE(parseBinanceTradeMessage(json).has_value());
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_missingTradeTimeReturnsNullopt) {
    const std::string json = R"({"e":"trade","s":"BTCUSDT","p":"50000","q":"1","m":true})";
    EXPECT_FALSE(parseBinanceTradeMessage(json).has_value());
}

TEST(BinanceTradeMessageParserTest,
     test_parseBinanceTradeMessage_missingIsBuyerMakerReturnsNullopt) {
    const std::string json = R"({"e":"trade","s":"BTCUSDT","p":"50000","q":"1","T":0})";
    EXPECT_FALSE(parseBinanceTradeMessage(json).has_value());
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_invalidJsonReturnsNullopt) {
    EXPECT_FALSE(parseBinanceTradeMessage("not json at all").has_value());
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_emptyStringReturnsNullopt) {
    EXPECT_FALSE(parseBinanceTradeMessage("").has_value());
}

TEST(BinanceTradeMessageParserTest, test_parseBinanceTradeMessage_emptyJsonObjectReturnsNullopt) {
    EXPECT_FALSE(parseBinanceTradeMessage("{}").has_value());
}

}  // namespace binance::test
