#pragma once

#include <cstdint>
#include <string>

namespace market {

/// @brief Normalized trade event received from an exchange market data stream.
struct TradeEvent {
    std::string symbol;
    double price{};
    double quantity{};
    bool isBuyerMaker{};
    int64_t tradeTimeMs{};
};

}  // namespace market
