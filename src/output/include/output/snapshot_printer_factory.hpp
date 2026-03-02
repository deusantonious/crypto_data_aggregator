#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "i_snapshot_printer.hpp"

namespace output {

enum class OutputFormat : std::uint8_t { PlainTextV1 };

/// @brief Factory for creating snapshot printer instances with different formats and output
/// targets.
class SnapshotPrinterFactory {
public:
    /// @brief Creates a file-backed snapshot printer with the specified format.
    /// @param format The output format to use (e.g. PlainTextV1).
    /// @param path   Filesystem path for the output file. Parent directories are created as needed.
    /// @return An owning pointer to the constructed ISnapshotPrinter.
    /// @throws std::invalid_argument if the output format is not supported.
    static std::unique_ptr<ISnapshotPrinter> createFileOutput(OutputFormat format,
                                                              const std::filesystem::path& path);
};

}  // namespace output
