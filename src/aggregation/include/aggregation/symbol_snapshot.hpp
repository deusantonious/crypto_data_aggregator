#pragma once

#include <cstdint>
#include <string>

namespace aggregation {

/// @brief Per-symbol trade statistics accumulated over a single aggregation window.
struct SymbolSnapshot {
    std::string symbol;
    uint64_t tradeCount{0};
    double totalVolume{0.0};
    double minPrice{0.0};
    double maxPrice{0.0};
    uint64_t buyCount{0};
    uint64_t sellCount{0};
};

}  // namespace aggregation
