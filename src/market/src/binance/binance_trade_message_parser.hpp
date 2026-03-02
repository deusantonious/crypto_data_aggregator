#pragma once

#include <market/trade_event.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace binance {

/// @brief Parses a raw Binance WebSocket JSON message into a TradeEvent.
/// @param raw The raw JSON string received from the Binance WebSocket stream.
/// @return A TradeEvent if the message is a valid trade, or std::nullopt on parse failure
///         or non-trade message types.
inline std::optional<market::TradeEvent> parseBinanceTradeMessage(const std::string& raw) {
    try {
        auto json = nlohmann::json::parse(raw);

        if (!json.contains("e") || json["e"] != "trade") {
            return std::nullopt;
        }

        market::TradeEvent event;
        event.symbol = json.at("s").get<std::string>();
        event.price = std::stod(json.at("p").get<std::string>());
        event.quantity = std::stod(json.at("q").get<std::string>());
        event.isBuyerMaker = json.at("m").get<bool>();
        event.tradeTimeMs = json.at("T").get<int64_t>();

        return event;
    } catch (...) {
        return std::nullopt;
    }
}

}  // namespace binance
