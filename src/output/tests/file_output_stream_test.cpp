#include "file_output_stream.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace output::test {

class FileOutputStreamTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_testDir = std::filesystem::temp_directory_path() / "output_test";
        std::filesystem::create_directories(m_testDir);
        m_testFile = m_testDir / "test_output.txt";

        std::filesystem::remove(m_testFile);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(m_testDir, ec);
    }

    static std::string readFile(const std::filesystem::path& path) {
        std::ifstream ifs(path);
        std::ostringstream oss;
        oss << ifs.rdbuf();
        return oss.str();
    }

    std::filesystem::path m_testDir;
    std::filesystem::path m_testFile;
};

TEST_F(FileOutputStreamTest, test_write_dataWrittenToFile) {
    FileOutputStream stream(m_testFile);
    stream.write("hello world\n");

    EXPECT_EQ(readFile(m_testFile), "hello world\n");
}

TEST_F(FileOutputStreamTest, test_write_appendsMultipleWrites) {
    FileOutputStream stream(m_testFile);
    stream.write("first\n");
    stream.write("second\n");

    EXPECT_EQ(readFile(m_testFile), "first\nsecond\n");
}

TEST_F(FileOutputStreamTest, test_write_createsParentDirectories) {
    auto nested = m_testDir / "a" / "b" / "c" / "nested.txt";

    FileOutputStream stream(nested);
    stream.write("nested\n");

    EXPECT_EQ(readFile(nested), "nested\n");
}

TEST_F(FileOutputStreamTest, test_write_emptyPathDoesNotCrash) {
    FileOutputStream stream(std::filesystem::path{});
    EXPECT_NO_THROW(stream.write("data"));
}

TEST_F(FileOutputStreamTest, test_write_emptyWriteCreatesFile) {
    FileOutputStream stream(m_testFile);
    stream.write("");

    EXPECT_TRUE(std::filesystem::exists(m_testFile));
    EXPECT_EQ(readFile(m_testFile), "");
}

}  // namespace output::test
