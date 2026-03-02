#pragma once

#include <format>
#include <market/market_streams_controller.hpp>
#include <span>
#include <string>
#include <string_view>

namespace binance {

constexpr std::string_view kWsHost = "stream.binance.com";
constexpr std::string_view kWsPort = "9443";
constexpr std::string_view kWsPath = "/ws";

/// @brief Converts a CurrencyPair enum value to its lowercase Binance symbol string.
/// @param pair The currency pair to convert.
/// @return A string_view such as "btcusdt", "ethusdt", etc.
constexpr std::string_view toSymbol(market::CurrencyPair pair) {
    using enum market::CurrencyPair;
    switch (pair) {
        case BTCUSDT:
            return "btcusdt";
        case ETHUSDT:
            return "ethusdt";
        case BNBUSDT:
            return "bnbusdt";
        case XRPUSDT:
            return "xrpusdt";
        case SOLUSDT:
            return "solusdt";
    }
    return "";
}

/// @brief Builds a combined Binance WebSocket stream path for the given currency pairs.
/// @param pairs The currency pairs to subscribe to (e.g. BTCUSDT, ETHUSDT).
/// @return A path string such as "/ws/btcusdt@trade/ethusdt@trade".
inline std::string buildStreamPath(std::span<const market::CurrencyPair> pairs) {
    std::string path{kWsPath};
    for (const auto& pair : pairs) {
        path += std::format("/{}@trade", toSymbol(pair));
    }
    return path;
}

}  // namespace binance
