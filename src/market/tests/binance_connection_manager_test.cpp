#include "binance/binance_connection_manager.hpp"

#include <gtest/gtest.h>

#include <boost/asio/io_context.hpp>
#include <memory>
#include <string>

namespace market::test {

class BinanceConnectionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_manager = std::make_shared<BinanceConnectionManager>(m_ioCtx, "stream.binance.com", "443",
                                                               "/ws/btcusdt@trade");
    }

    void TearDown() override {
        if (m_manager) {
            m_manager->disconnect();
        }
        m_ioCtx.stop();
    }

    boost::asio::io_context m_ioCtx;
    std::shared_ptr<BinanceConnectionManager> m_manager;
};

// ---------- Initial state ----------

TEST_F(BinanceConnectionManagerTest, test_isConnected_initiallyFalse) {
    EXPECT_FALSE(m_manager->isConnected());
}

// ---------- disconnect lifecycle ----------

TEST_F(BinanceConnectionManagerTest, test_disconnect_beforeConnectDoesNotThrow) {
    EXPECT_NO_THROW(m_manager->disconnect());
}

TEST_F(BinanceConnectionManagerTest, test_disconnect_doubleDisconnectDoesNotThrow) {
    m_manager->disconnect();
    EXPECT_NO_THROW(m_manager->disconnect());
}

TEST_F(BinanceConnectionManagerTest, test_disconnect_afterConnectDoesNotThrow) {
    m_manager->connect();
    EXPECT_NO_THROW(m_manager->disconnect());
    EXPECT_FALSE(m_manager->isConnected());
}

// ---------- connect lifecycle ----------

TEST_F(BinanceConnectionManagerTest, test_connect_doesNotThrow) {
    EXPECT_NO_THROW(m_manager->connect());
    m_manager->disconnect();
}

TEST_F(BinanceConnectionManagerTest, test_connect_doubleConnectDoesNotThrow) {
    m_manager->connect();
    EXPECT_NO_THROW(m_manager->connect());
    m_manager->disconnect();
}

TEST_F(BinanceConnectionManagerTest, test_connect_isNotImmediatelyConnected) {
    m_manager->connect();
    // Connection is asynchronous; should not be connected yet without running io_context.
    EXPECT_FALSE(m_manager->isConnected());
}

// ---------- setMessageCallback ----------

TEST_F(BinanceConnectionManagerTest, test_setMessageCallback_beforeConnect) {
    bool called = false;
    EXPECT_NO_THROW(m_manager->setMessageCallback([&](const std::string&) { called = true; }));
    EXPECT_FALSE(called);
}

TEST_F(BinanceConnectionManagerTest, test_setMessageCallback_nullCallbackDoesNotThrow) {
    EXPECT_NO_THROW(m_manager->setMessageCallback(nullptr));
}

TEST_F(BinanceConnectionManagerTest, test_setMessageCallback_replaceCallbackDoesNotThrow) {
    m_manager->setMessageCallback([](const std::string&) {});
    EXPECT_NO_THROW(m_manager->setMessageCallback([](const std::string&) {}));
}

// ---------- Destruction safety ----------

TEST_F(BinanceConnectionManagerTest, test_destructor_whileConnectingDoesNotThrow) {
    m_manager->connect();
    EXPECT_NO_THROW(m_manager.reset());
}

TEST_F(BinanceConnectionManagerTest, test_destructor_afterDisconnectDoesNotThrow) {
    m_manager->disconnect();
    EXPECT_NO_THROW(m_manager.reset());
}

// ---------- Reconnect after failed resolve ----------

TEST_F(BinanceConnectionManagerTest, test_connect_failedResolveSchedulesReconnect) {
    // Use an invalid host to trigger a resolve failure.
    auto badManager = std::make_shared<BinanceConnectionManager>(
        m_ioCtx, "this.host.does.not.exist.invalid", "443", "/ws");

    badManager->connect();

    // Run io_context briefly to process the async resolve (which should fail).
    m_ioCtx.run_for(std::chrono::seconds(3));

    // Should not be connected after a failed resolve.
    EXPECT_FALSE(badManager->isConnected());

    badManager->disconnect();
}

}  // namespace market::test
