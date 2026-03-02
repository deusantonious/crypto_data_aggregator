#pragma once

#include <aggregation/symbol_snapshot.hpp>
#include <cstdint>
#include <limits>
#include <string>

namespace aggregation {

/// @brief Accumulates trade statistics (count, volume, price range, buy/sell counts)
///        for a single symbol within one aggregation window.
class WindowStats {
public:
    /// @brief Records a single trade, updating all tracked statistics.
    /// @param price    Trade execution price.
    /// @param quantity Trade execution quantity.
    /// @param isBuyerMaker True if the buyer was the maker (i.e. a sell-side aggressor trade).
    void update(double price, double quantity, bool isBuyerMaker);

    /// @brief Resets all statistics to their initial state for the next window.
    void reset();

    /// @brief Creates an immutable snapshot of the current statistics.
    /// @param symbol The trading pair symbol to tag the snapshot with.
    /// @return A SymbolSnapshot containing a copy of all accumulated statistics.
    [[nodiscard]] SymbolSnapshot toSnapshot(const std::string& symbol) const;

    [[nodiscard]] uint64_t tradeCount() const {
        return m_tradeCount;
    }
    [[nodiscard]] double totalVolume() const {
        return m_totalVolume;
    }
    [[nodiscard]] double minPrice() const {
        return m_minPrice;
    }
    [[nodiscard]] double maxPrice() const {
        return m_maxPrice;
    }
    [[nodiscard]] uint64_t buyCount() const {
        return m_buyCount;
    }
    [[nodiscard]] uint64_t sellCount() const {
        return m_sellCount;
    }

private:
    uint64_t m_tradeCount{0};
    double m_totalVolume{0.0};
    double m_minPrice{std::numeric_limits<double>::max()};
    double m_maxPrice{std::numeric_limits<double>::lowest()};
    uint64_t m_buyCount{0};
    uint64_t m_sellCount{0};
};

}  // namespace aggregation
