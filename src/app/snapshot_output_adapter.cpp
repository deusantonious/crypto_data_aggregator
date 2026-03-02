#include "snapshot_output_adapter.hpp"

namespace app {

std::vector<output::SymbolSnapshot> adaptAggregationSnapshotsToOutput(
    const std::vector<aggregation::SymbolSnapshot>& snapshots) {
    std::vector<output::SymbolSnapshot> result;
    result.reserve(snapshots.size());

    for (const auto& snapshot : snapshots) {
        result.push_back(output::SymbolSnapshot{
            .symbol = snapshot.symbol,
            .tradeCount = snapshot.tradeCount,
            .totalVolume = snapshot.totalVolume,
            .minPrice = snapshot.minPrice,
            .maxPrice = snapshot.maxPrice,
            .buyCount = snapshot.buyCount,
            .sellCount = snapshot.sellCount,
        });
    }

    return result;
}

}  // namespace app
