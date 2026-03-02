#include "binance/binance_config.hpp"

#include <gtest/gtest.h>

#include <array>
#include <string>

namespace binance::test {

TEST(BinanceConfigTest, test_toSymbol_btcusdt) {
    EXPECT_EQ(toSymbol(market::CurrencyPair::BTCUSDT), "btcusdt");
}

TEST(BinanceConfigTest, test_toSymbol_ethusdt) {
    EXPECT_EQ(toSymbol(market::CurrencyPair::ETHUSDT), "ethusdt");
}

TEST(BinanceConfigTest, test_toSymbol_bnbusdt) {
    EXPECT_EQ(toSymbol(market::CurrencyPair::BNBUSDT), "bnbusdt");
}

TEST(BinanceConfigTest, test_toSymbol_xrpusdt) {
    EXPECT_EQ(toSymbol(market::CurrencyPair::XRPUSDT), "xrpusdt");
}

TEST(BinanceConfigTest, test_toSymbol_solusdt) {
    EXPECT_EQ(toSymbol(market::CurrencyPair::SOLUSDT), "solusdt");
}

TEST(BinanceConfigTest, test_buildStreamPath_singlePair) {
    const std::array pairs = {market::CurrencyPair::BTCUSDT};
    EXPECT_EQ(buildStreamPath(pairs), "/ws/btcusdt@trade");
}

TEST(BinanceConfigTest, test_buildStreamPath_multiplePairs) {
    const std::array pairs = {
        market::CurrencyPair::BTCUSDT,
        market::CurrencyPair::ETHUSDT,
        market::CurrencyPair::SOLUSDT,
    };
    EXPECT_EQ(buildStreamPath(pairs), "/ws/btcusdt@trade/ethusdt@trade/solusdt@trade");
}

TEST(BinanceConfigTest, test_buildStreamPath_empty) {
    const std::array<market::CurrencyPair, 0> pairs = {};
    EXPECT_EQ(buildStreamPath(pairs), "/ws");
}

TEST(BinanceConfigTest, test_buildStreamPath_allPairs) {
    const std::array pairs = {
        market::CurrencyPair::BTCUSDT, market::CurrencyPair::ETHUSDT, market::CurrencyPair::BNBUSDT,
        market::CurrencyPair::XRPUSDT, market::CurrencyPair::SOLUSDT,
    };
    EXPECT_EQ(buildStreamPath(pairs),
              "/ws/btcusdt@trade/ethusdt@trade/bnbusdt@trade/xrpusdt@trade/solusdt@trade");
}

TEST(BinanceConfigTest, test_constants_wsHostValue) {
    EXPECT_EQ(kWsHost, "stream.binance.com");
}

TEST(BinanceConfigTest, test_constants_wsPortValue) {
    EXPECT_EQ(kWsPort, "9443");
}

TEST(BinanceConfigTest, test_constants_wsPathValue) {
    EXPECT_EQ(kWsPath, "/ws");
}

}  // namespace binance::test
