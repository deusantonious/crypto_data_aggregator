#include "formatted_snapshot_printer.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <output/i_snapshot_printer.hpp>
#include <string>
#include <vector>

#include "mocks/mock_output_stream.hpp"
#include "mocks/mock_snapshot_formatter.hpp"

namespace output::test {

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

class FormattedSnapshotPrinterTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto formatterOwner = std::make_unique<StrictMock<MockSnapshotFormatter>>();
        auto streamOwner = std::make_unique<StrictMock<MockOutputStream>>();

        formatter = formatterOwner.get();
        stream = streamOwner.get();

        printer = std::make_unique<FormattedSnapshotPrinter>(std::move(formatterOwner),
                                                             std::move(streamOwner));
    }

    StrictMock<MockSnapshotFormatter>* formatter{};
    StrictMock<MockOutputStream>* stream{};
    std::unique_ptr<FormattedSnapshotPrinter> printer;
};

TEST_F(FormattedSnapshotPrinterTest, test_write_delegatesToFormatterAndStream) {
    const std::string expectedFormatted = "formatted output\n";

    EXPECT_CALL(*formatter, format(_, 12345)).WillOnce(Return(expectedFormatted));
    EXPECT_CALL(*stream, write(expectedFormatted)).Times(1);

    std::vector<SymbolSnapshot> snapshots = {{.symbol = "BTCUSDT", .tradeCount = 1}};
    printer->write(snapshots, 12345);
}

TEST_F(FormattedSnapshotPrinterTest, test_write_emptySnapshotsStillCallsFormatterAndStream) {
    EXPECT_CALL(*formatter, format(_, _)).WillOnce(Return("empty\n"));
    EXPECT_CALL(*stream, write("empty\n")).Times(1);

    std::vector<SymbolSnapshot> empty;
    printer->write(empty, 0);
}

TEST_F(FormattedSnapshotPrinterTest, test_write_multipleWriteCalls) {
    EXPECT_CALL(*formatter, format(_, _)).Times(3).WillRepeatedly(Return("data\n"));
    EXPECT_CALL(*stream, write("data\n")).Times(3);

    std::vector<SymbolSnapshot> snapshots = {{.symbol = "X"}};
    printer->write(snapshots, 1);
    printer->write(snapshots, 2);
    printer->write(snapshots, 3);
}

}  // namespace output::test
