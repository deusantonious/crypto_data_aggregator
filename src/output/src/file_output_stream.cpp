#include "file_output_stream.hpp"

#include <spdlog/spdlog.h>

#include <system_error>
#include <utility>

namespace output {

FileOutputStream::FileOutputStream(std::filesystem::path path) noexcept : m_path(std::move(path)) {}

bool FileOutputStream::ensureFileReady() noexcept {
    if (m_file.is_open()) {
        return true;
    }

    if (m_path.empty()) {
        spdlog::error("Output file path is empty");
        return false;
    }

    const auto parent = m_path.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            spdlog::error("Failed to create output directory '{}': {}", parent.string(),
                          ec.message());
            return false;
        }
    }

    m_file.open(m_path, std::ios::app);
    if (!m_file.is_open()) {
        spdlog::error("Failed to open output file '{}'", m_path.string());
        return false;
    }

    return true;
}

void FileOutputStream::write(const std::string& data) noexcept {
    if (!ensureFileReady()) {
        return;
    }

    m_file << data;
    if (!m_file) {
        spdlog::error("Failed to write to output file '{}'", m_path.string());
        m_file.clear();
        return;
    }

    m_file.flush();
    if (!m_file) {
        spdlog::error("Failed to flush output file '{}'", m_path.string());
        m_file.clear();
    }
}

}  // namespace output
