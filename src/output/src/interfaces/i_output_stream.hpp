#pragma once

#include <string>

namespace output {

class IOutputStream {
public:
    virtual ~IOutputStream() = default;
    IOutputStream(const IOutputStream&) = delete;
    IOutputStream& operator=(const IOutputStream&) = delete;
    IOutputStream(IOutputStream&&) = delete;
    IOutputStream& operator=(IOutputStream&&) = delete;

    /// @brief Writes the given data string to the output destination.
    /// @param data The formatted string to write.
    virtual void write(const std::string& data) = 0;

protected:
    IOutputStream() = default;
};

}  // namespace output
