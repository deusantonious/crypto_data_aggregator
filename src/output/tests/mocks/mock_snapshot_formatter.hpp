#pragma once

#include <gmock/gmock.h>

#include "interfaces/i_snapshot_formatter.hpp"

namespace output::test {

class MockSnapshotFormatter : public ISnapshotFormatter {
public:
    MOCK_METHOD(std::string, format,
                (const std::vector<output::SymbolSnapshot>& snapshots, int64_t timestamp),
                (override));
};

}  // namespace output::test
