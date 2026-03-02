#pragma once

#include "interfaces/i_snapshot_formatter.hpp"

namespace output {

/// @brief Formats snapshots as human-readable plain text with UTC ISO-8601 timestamps.
class PlainTextFormatter : public ISnapshotFormatter {
public:
    /// @brief Formats snapshots into a plain-text block with one line per symbol.
    /// @param snapshots Per-symbol trade statistics to format.
    /// @param timestamp Unix timestamp in milliseconds converted to UTC ISO-8601 in the output.
    /// @return A multi-line plain-text string suitable for file or console output.
    std::string format(const std::vector<output::SymbolSnapshot>& snapshots,
                       int64_t timestamp) override;
};

}  // namespace output
