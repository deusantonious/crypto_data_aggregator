#include "app/config_loader.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <market/market_streams_controller.hpp>
#include <stdexcept>
#include <string>

namespace app::test {

namespace {

class ConfigLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempDir = std::filesystem::temp_directory_path() / "config_test";
        std::filesystem::create_directories(m_tempDir);
        m_configPath = m_tempDir / "config.json";
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(m_tempDir, ec);
    }

    void writeConfig(const std::string& content) const {
        std::ofstream ofs(m_configPath);
        ofs << content;
    }

    std::filesystem::path m_tempDir;
    std::filesystem::path m_configPath;
};

}  // namespace

TEST_F(ConfigLoaderTest, test_loadFromFile_loadsValidConfig) {
    writeConfig(R"({
        "windowLength": 5,
        "outputPath": "./output.txt",
        "exchanges": [
            {
                "name": "binance",
                "streams": ["BTCUSDT", "ETHUSDT"]
            }
        ]
    })");

    auto config = ConfigLoader::loadFromFile(m_configPath);

    EXPECT_EQ(config.windowLength, 5);
    EXPECT_EQ(config.outputPath, "./output.txt");
    ASSERT_EQ(config.exchanges.size(), 1U);
    EXPECT_EQ(config.exchanges[0].exchange, market::Exchange::Binance);
    ASSERT_EQ(config.exchanges[0].streams.size(), 2U);
    EXPECT_EQ(config.exchanges[0].streams[0], market::CurrencyPair::BTCUSDT);
    EXPECT_EQ(config.exchanges[0].streams[1], market::CurrencyPair::ETHUSDT);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_allSupportedStreams) {
    writeConfig(R"({
        "windowLength": 10,
        "exchanges": [
            {
                "name": "binance",
                "streams": ["BTCUSDT", "ETHUSDT", "BNBUSDT", "XRPUSDT", "SOLUSDT"]
            }
        ]
    })");

    auto config = ConfigLoader::loadFromFile(m_configPath);

    ASSERT_EQ(config.exchanges[0].streams.size(), 5U);
    EXPECT_EQ(config.exchanges[0].streams[0], market::CurrencyPair::BTCUSDT);
    EXPECT_EQ(config.exchanges[0].streams[1], market::CurrencyPair::ETHUSDT);
    EXPECT_EQ(config.exchanges[0].streams[2], market::CurrencyPair::BNBUSDT);
    EXPECT_EQ(config.exchanges[0].streams[3], market::CurrencyPair::XRPUSDT);
    EXPECT_EQ(config.exchanges[0].streams[4], market::CurrencyPair::SOLUSDT);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_defaultOutputPath) {
    writeConfig(R"({
        "windowLength": 5,
        "exchanges": [
            {
                "name": "binance",
                "streams": ["BTCUSDT"]
            }
        ]
    })");

    auto config = ConfigLoader::loadFromFile(m_configPath);

    EXPECT_EQ(config.outputPath, "./output.txt");
}

TEST_F(ConfigLoaderTest, test_loadFromFile_nonexistentFileThrows) {
    EXPECT_THROW(ConfigLoader::loadFromFile("/nonexistent/path/config.json"), std::runtime_error);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_malformedJsonThrows) {
    writeConfig("{ not valid json }}}");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::runtime_error);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_missingWindowLengthThrows) {
    writeConfig(R"({
        "exchanges": [{"name": "binance", "streams": ["BTCUSDT"]}]
    })");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::exception);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_missingExchangesThrows) {
    writeConfig(R"({"windowLength": 5})");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::exception);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_emptyExchangesArrayThrows) {
    writeConfig(R"({"windowLength": 5, "exchanges": []})");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::runtime_error);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_zeroWindowLengthThrows) {
    writeConfig(R"({
        "windowLength": 0,
        "exchanges": [{"name": "binance", "streams": ["BTCUSDT"]}]
    })");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::runtime_error);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_negativeWindowLengthThrows) {
    writeConfig(R"({
        "windowLength": -1,
        "exchanges": [{"name": "binance", "streams": ["BTCUSDT"]}]
    })");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::runtime_error);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_unsupportedExchangeThrows) {
    writeConfig(R"({
        "windowLength": 5,
        "exchanges": [{"name": "kraken", "streams": ["BTCUSDT"]}]
    })");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::runtime_error);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_unsupportedStreamThrows) {
    writeConfig(R"({
        "windowLength": 5,
        "exchanges": [{"name": "binance", "streams": ["INVALID"]}]
    })");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::runtime_error);
}

TEST_F(ConfigLoaderTest, test_loadFromFile_emptyStreamsArrayThrows) {
    writeConfig(R"({
        "windowLength": 5,
        "exchanges": [{"name": "binance", "streams": []}]
    })");
    EXPECT_THROW(ConfigLoader::loadFromFile(m_configPath), std::runtime_error);
}

}  // namespace app::test
