#pragma once

#include <gmock/gmock.h>

#include "interfaces/i_exchange_connection_manager.hpp"

namespace market::test {

class MockExchangeConnectionManager : public IExchangeConnectionManager {
public:
    MOCK_METHOD(void, connect, (), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(void, setMessageCallback, (MessageCallback callback), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
};

}  // namespace market::test
