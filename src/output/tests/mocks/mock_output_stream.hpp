#pragma once

#include <gmock/gmock.h>

#include "interfaces/i_output_stream.hpp"

namespace output::test {

class MockOutputStream : public IOutputStream {
public:
    MOCK_METHOD(void, write, (const std::string& data), (override));
};

}  // namespace output::test
