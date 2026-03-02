#pragma once

#include <aggregation/symbol_snapshot.hpp>
#include <output/i_snapshot_printer.hpp>
#include <vector>

namespace app {

/// @brief Converts aggregation-layer snapshots to output-layer snapshots.
/// @param snapshots Source snapshots from the aggregation subsystem.
/// @return A vector of output::SymbolSnapshot objects with identical field values.
std::vector<output::SymbolSnapshot> adaptAggregationSnapshotsToOutput(
    const std::vector<aggregation::SymbolSnapshot>& snapshots);

}  // namespace app
