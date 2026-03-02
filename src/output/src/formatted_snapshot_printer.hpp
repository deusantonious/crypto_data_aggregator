#pragma once

#include <memory>
#include <output/i_snapshot_printer.hpp>
#include <string>
#include <utility>

#include "interfaces/i_output_stream.hpp"
#include "interfaces/i_snapshot_formatter.hpp"

namespace output {

/// @brief Snapshot printer that delegates formatting and writing to injected strategies.
class FormattedSnapshotPrinter : public ISnapshotPrinter {
public:
    /// @brief Constructs a printer with the given formatter and output stream.
    /// @param formatter Strategy used to convert snapshots into a formatted string.
    /// @param stream    Output stream to which the formatted data is written.
    FormattedSnapshotPrinter(std::unique_ptr<ISnapshotFormatter> formatter,
                             std::unique_ptr<IOutputStream> stream)
        : m_formatter(std::move(formatter)), m_stream(std::move(stream)) {}

    /// @brief Formats the snapshots and writes them to the underlying output stream.
    void write(const std::vector<output::SymbolSnapshot>& snapshots, int64_t timestamp) override {
        auto formatted = m_formatter->format(snapshots, timestamp);
        m_stream->write(formatted);
    }

private:
    std::unique_ptr<ISnapshotFormatter> m_formatter;
    std::unique_ptr<IOutputStream> m_stream;
};

}  // namespace output
