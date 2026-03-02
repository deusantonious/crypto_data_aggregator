#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include "interfaces/i_output_stream.hpp"

namespace output {

/// @brief IOutputStream implementation that appends data to a file on disk.
///        Creates parent directories on first write if they do not exist.
class FileOutputStream : public IOutputStream {
public:
    /// @brief Constructs a file output stream targeting the given path.
    /// @param path Filesystem path for the output file. The file is opened lazily on first write.
    explicit FileOutputStream(std::filesystem::path path) noexcept;

    /// @brief Appends data to the output file. Logs errors on I/O failure instead of throwing.
    /// @param data The string content to append.
    void write(const std::string& data) noexcept override;

private:
    /// @brief Lazily opens the output file, creating parent directories if necessary.
    /// @return True if the file is open and ready for writing, false on failure.
    bool ensureFileReady() noexcept;

    std::filesystem::path m_path;
    std::ofstream m_file;
};

}  // namespace output
