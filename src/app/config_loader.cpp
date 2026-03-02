#include "config_loader.hpp"

#include <filesystem>
#include <fstream>
#include <market/market_streams_controller.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application.hpp"

namespace app {
namespace {

market::Exchange parseExchange(std::string_view name) {
    if (name == "binance") {
        return market::Exchange::Binance;
    }
    throw std::runtime_error("Unsupported exchange: " + std::string(name));
}

market::CurrencyPair parseStream(std::string_view stream) {
    if (stream == "BTCUSDT") {
        return market::CurrencyPair::BTCUSDT;
    }
    if (stream == "ETHUSDT") {
        return market::CurrencyPair::ETHUSDT;
    }
    if (stream == "BNBUSDT") {
        return market::CurrencyPair::BNBUSDT;
    }
    if (stream == "XRPUSDT") {
        return market::CurrencyPair::XRPUSDT;
    }
    if (stream == "SOLUSDT") {
        return market::CurrencyPair::SOLUSDT;
    }
    throw std::runtime_error("Unsupported stream: " + std::string(stream));
}

}  // namespace

ApplicationConfig ConfigLoader::loadFromFile(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Failed to open config file: " + path.string());
    }

    nlohmann::json json;
    try {
        input >> json;
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Invalid JSON config: " + std::string(e.what()));
    }

    ApplicationConfig config;
    config.windowLength = json.at("windowLength").get<int>();
    config.outputPath = std::filesystem::path(json.value("outputPath", "./output.txt"));

    const auto& exchanges = json.at("exchanges");
    if (!exchanges.is_array() || exchanges.empty()) {
        throw std::runtime_error("Config field 'exchanges' must be a non-empty array");
    }

    for (const auto& entry : exchanges) {
        const auto exchangeName = entry.at("name").get<std::string>();
        auto exchange = parseExchange(exchangeName);

        const auto& streamsJson = entry.at("streams");
        if (!streamsJson.is_array() || streamsJson.empty()) {
            throw std::runtime_error("Exchange '" + exchangeName + "' has empty streams list");
        }

        ExchangeStreamsConfig exchangeConfig;
        exchangeConfig.exchange = exchange;

        for (const auto& streamValue : streamsJson) {
            exchangeConfig.streams.push_back(parseStream(streamValue.get<std::string>()));
        }

        config.exchanges.push_back(std::move(exchangeConfig));
    }

    if (config.windowLength <= 0) {
        throw std::runtime_error("Config field 'windowLength' must be > 0");
    }

    return config;
}

}  // namespace app
