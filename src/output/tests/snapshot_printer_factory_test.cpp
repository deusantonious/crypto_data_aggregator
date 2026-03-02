#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <output/i_snapshot_printer.hpp>
#include <output/snapshot_printer_factory.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "plain_text_formatter.hpp"

namespace output::test {

class SnapshotPrinterFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_testDir = std::filesystem::temp_directory_path() / "factory_test";
        std::filesystem::create_directories(m_testDir);
        m_testFile = m_testDir / "output.txt";
        std::filesystem::remove(m_testFile);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(m_testDir, ec);
    }

    static std::string readFile(const std::filesystem::path& path) {
        std::ifstream ifs(path);
        std::ostringstream oss;
        oss << ifs.rdbuf();
        return oss.str();
    }

    std::filesystem::path m_testDir;
    std::filesystem::path m_testFile;
};

TEST_F(SnapshotPrinterFactoryTest, test_createFileOutput_plainTextV1ReturnsNonNull) {
    auto result = SnapshotPrinterFactory::createFileOutput(OutputFormat::PlainTextV1, m_testFile);
    EXPECT_NE(result, nullptr);
}

TEST_F(SnapshotPrinterFactoryTest, test_createFileOutput_plainTextV1OutputMatchesFormatter) {
    auto printer = SnapshotPrinterFactory::createFileOutput(OutputFormat::PlainTextV1, m_testFile);

    std::vector<SymbolSnapshot> snapshots = {
        {.symbol = "BTCUSDT",
         .tradeCount = 1,
         .totalVolume = 50000.0,
         .minPrice = 50000.0,
         .maxPrice = 50000.0,
         .buyCount = 1,
         .sellCount = 0},
    };
    constexpr int64_t timestamp = 1700000000000;

    printer->write(snapshots, timestamp);

    PlainTextFormatter formatter;
    auto expected = formatter.format(snapshots, timestamp);

    EXPECT_EQ(readFile(m_testFile), expected);
}

TEST_F(SnapshotPrinterFactoryTest, test_createFileOutput_unsupportedFormatThrows) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    auto badFormat = static_cast<OutputFormat>(255);
    EXPECT_THROW(SnapshotPrinterFactory::createFileOutput(badFormat, m_testFile),
                 std::invalid_argument);
}

}  // namespace output::test
