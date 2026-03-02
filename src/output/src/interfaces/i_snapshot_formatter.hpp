#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace output {

struct SymbolSnapshot;

class ISnapshotFormatter {
public:
    virtual ~ISnapshotFormatter() = default;
    ISnapshotFormatter(const ISnapshotFormatter&) = delete;
    ISnapshotFormatter& operator=(const ISnapshotFormatter&) = delete;
    ISnapshotFormatter(ISnapshotFormatter&&) = delete;
    ISnapshotFormatter& operator=(ISnapshotFormatter&&) = delete;

    /// @brief Formats per-symbol snapshots and a timestamp into a printable string.
    /// @param snapshots Per-symbol trade statistics to format.
    /// @param timestamp Unix timestamp in milliseconds representing the window end time.
    /// @return A formatted string ready for output.
    virtual std::string format(const std::vector<output::SymbolSnapshot>& snapshots,
                               int64_t timestamp) = 0;

protected:
    ISnapshotFormatter() = default;
};

}  // namespace output
