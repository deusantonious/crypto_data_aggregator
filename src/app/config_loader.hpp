#pragma once

#include <filesystem>
#include <string>

#include "application.hpp"

namespace app {

class ConfigLoader {
public:
    /// @throws std::runtime_error on any config loading failure.
    static ApplicationConfig loadFromFile(const std::filesystem::path& path);
};

}  // namespace app
