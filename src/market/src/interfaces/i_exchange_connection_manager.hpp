#pragma once

#include <functional>
#include <string>

namespace market {

class IExchangeConnectionManager {
public:
    using MessageCallback = std::function<void(const std::string&)>;

    virtual ~IExchangeConnectionManager() = default;
    IExchangeConnectionManager(const IExchangeConnectionManager&) = delete;
    IExchangeConnectionManager& operator=(const IExchangeConnectionManager&) = delete;
    IExchangeConnectionManager(IExchangeConnectionManager&&) = delete;
    IExchangeConnectionManager& operator=(IExchangeConnectionManager&&) = delete;

    /// @brief Initiates an asynchronous connection to the exchange.
    virtual void connect() = 0;

    /// @brief Disconnects from the exchange and cancels any pending operations.
    virtual void disconnect() = 0;

    /// @brief Sets the callback invoked for each raw message received from the exchange.
    /// @param callback The handler for incoming messages, or nullptr to unsubscribe.
    virtual void setMessageCallback(MessageCallback callback) = 0;

    /// @brief Returns whether the connection is currently established.
    /// @return True if connected, false otherwise.
    [[nodiscard]] virtual bool isConnected() const = 0;

protected:
    IExchangeConnectionManager() = default;
};

}  // namespace market
