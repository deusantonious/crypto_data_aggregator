#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace output {

/// @brief Per-symbol trade statistics used by the output layer.
struct SymbolSnapshot {
    std::string symbol;
    uint64_t tradeCount{0};
    double totalVolume{0.0};
    double minPrice{0.0};
    double maxPrice{0.0};
    uint64_t buyCount{0};
    uint64_t sellCount{0};
};

class ISnapshotPrinter {
public:
    virtual ~ISnapshotPrinter() = default;
    ISnapshotPrinter(const ISnapshotPrinter&) = delete;
    ISnapshotPrinter& operator=(const ISnapshotPrinter&) = delete;
    ISnapshotPrinter(ISnapshotPrinter&&) = delete;
    ISnapshotPrinter& operator=(ISnapshotPrinter&&) = delete;

    /// @brief Formats and writes the given snapshots to the output destination.
    /// @param snapshots Per-symbol statistics to output.
    /// @param timestamp Unix timestamp in milliseconds representing the window end time.
    virtual void write(const std::vector<output::SymbolSnapshot>& snapshots, int64_t timestamp) = 0;

protected:
    ISnapshotPrinter() = default;
};

}  // namespace output
