#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <filesystem>

#include "app/application.hpp"
#include "app/config_loader.hpp"

int main() {
    spdlog::set_level(spdlog::level::debug);

    try {
        const std::filesystem::path configPath{"config.json"};
        auto config = app::ConfigLoader::loadFromFile(configPath);

        app::Application app(config);
        app.run();
    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }

    return 0;
}
