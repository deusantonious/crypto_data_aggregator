#include "plain_text_formatter.hpp"

#include <array>
#include <ctime>
#include <format>
#include <output/i_snapshot_printer.hpp>
#include <string>

namespace output {

namespace {

std::string toUtcIso8601(int64_t timestampMs) {
    const auto timestampSeconds = static_cast<std::time_t>(timestampMs / 1000);

    std::tm utcTime{};
    gmtime_r(&timestampSeconds, &utcTime);

    std::array<char, 32> buffer{};
    std::strftime(buffer.data(), buffer.size(), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
    return {buffer.data()};
}

/// Formats a double with up to 10 significant digits, guaranteeing at least
/// one digit after the decimal point (e.g. 1000 → "1000.0", 23.51 → "23.51").
std::string formatDecimal(double value) {
    auto str = std::format("{:.10g}", value);
    if (str.find('.') == std::string::npos && str.find('e') == std::string::npos) {
        str += ".0";
    }
    return str;
}

}  // namespace

std::string PlainTextFormatter::format(const std::vector<output::SymbolSnapshot>& snapshots,
                                       int64_t timestamp) {
    std::string result = std::format("timestamp={}\n", toUtcIso8601(timestamp));

    for (const auto& snap : snapshots) {
        result += std::format("symbol={} trades={} volume={} min={} max={} buy={} sell={}\n",
                              snap.symbol, snap.tradeCount, formatDecimal(snap.totalVolume),
                              formatDecimal(snap.minPrice), formatDecimal(snap.maxPrice),
                              snap.buyCount, snap.sellCount);
    }
    result += "\n";
    return result;
}

}  // namespace output
