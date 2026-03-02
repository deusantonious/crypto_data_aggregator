#include "window_stats.hpp"

#include <algorithm>

namespace aggregation {

void WindowStats::update(double price, double quantity, bool isBuyerMaker) {
    m_tradeCount++;
    m_totalVolume += price * quantity;
    m_minPrice = std::min(m_minPrice, price);
    m_maxPrice = std::max(m_maxPrice, price);

    if (isBuyerMaker) {
        m_sellCount++;  // buyer is maker = sell order was aggressor
    } else {
        m_buyCount++;  // seller is maker = buy order was aggressor
    }
}

void WindowStats::reset() {
    m_tradeCount = 0;
    m_totalVolume = 0.0;
    m_minPrice = std::numeric_limits<double>::max();
    m_maxPrice = std::numeric_limits<double>::lowest();
    m_buyCount = 0;
    m_sellCount = 0;
}

SymbolSnapshot WindowStats::toSnapshot(const std::string& symbol) const {
    return {
        .symbol = symbol,
        .tradeCount = m_tradeCount,
        .totalVolume = m_totalVolume,
        .minPrice = m_minPrice,
        .maxPrice = m_maxPrice,
        .buyCount = m_buyCount,
        .sellCount = m_sellCount,
    };
}

}  // namespace aggregation
