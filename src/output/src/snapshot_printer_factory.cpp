#include <filesystem>
#include <memory>
#include <output/i_snapshot_printer.hpp>
#include <output/snapshot_printer_factory.hpp>
#include <stdexcept>
#include <utility>

#include "file_output_stream.hpp"
#include "formatted_snapshot_printer.hpp"
#include "interfaces/i_snapshot_formatter.hpp"
#include "plain_text_formatter.hpp"

namespace output {

std::unique_ptr<ISnapshotPrinter> SnapshotPrinterFactory::createFileOutput(
    OutputFormat format, const std::filesystem::path& path) {
    std::unique_ptr<ISnapshotFormatter> formatter;

    switch (format) {
        case OutputFormat::PlainTextV1:
            formatter = std::make_unique<PlainTextFormatter>();
            break;
        default:
            throw std::invalid_argument("Unsupported output format");
    }

    auto stream = std::make_unique<FileOutputStream>(path);

    return std::make_unique<FormattedSnapshotPrinter>(std::move(formatter), std::move(stream));
}

}  // namespace output
