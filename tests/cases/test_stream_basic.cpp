// tests/test_stream_basic.cpp
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>


#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"

namespace fs = std::filesystem;

class StreamBasicTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "wiseio_tests";
        fs::create_directories(test_dir_);
        logging::Logger::SetupLogger(logging::LoggerMode::kDebug, logging::LoggerIOMode::kSync, true);
    }

    void TearDown() override {
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }

    std::string CreateTestFile(const std::string& name, const std::string& content) {
        auto path = test_dir_ / name;
        std::ofstream file(path);
        file << content;
        file.close();
        return path.string();
    }

    fs::path test_dir_;
};

// ==================== Тесты на создание стрима ====================

TEST_F(StreamBasicTest, CreateStreamReadMode_Success) {
    auto path = CreateTestFile("test.txt", "Hello World");
    
    EXPECT_NO_THROW({
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    });
}

TEST_F(StreamBasicTest, CreateStreamWriteMode_Success) {
    auto path = (test_dir_ / "new_file.txt").string();
    
    EXPECT_NO_THROW({
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    });
}

TEST_F(StreamBasicTest, CreateStreamAppendMode_Success) {
    auto path = (test_dir_ / "append_file.txt").string();
    
    EXPECT_NO_THROW({
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
    });
}

TEST_F(StreamBasicTest, CreateStreamReadWriteMode_Success) {
    auto path = (test_dir_ / "rw_file.txt").string();
    
    EXPECT_NO_THROW({
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);
    });
}

TEST_F(StreamBasicTest, CreateStreamReadMode_FileNotExists_Throws) {
    auto path = (test_dir_ / "nonexistent.txt").string();
    
    EXPECT_THROW({
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    }, std::runtime_error);
}

// ==================== Тесты на перемещение ====================

TEST_F(StreamBasicTest, MoveConstructor_Success) {
    auto path = CreateTestFile("move_test.txt", "test");
    
    auto stream1 = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    wiseio::Stream stream2(std::move(stream1));
    
    // stream2 должен быть работоспособен
    EXPECT_FALSE(stream2.IsEOF());
}

TEST_F(StreamBasicTest, MoveAssignment_Success) {
    auto path1 = CreateTestFile("file1.txt", "test1");
    auto path2 = CreateTestFile("file2.txt", "test2");
    
    auto stream1 = wiseio::CreateStream(path1.c_str(), wiseio::OpenMode::kRead);
    auto stream2 = wiseio::CreateStream(path2.c_str(), wiseio::OpenMode::kRead);
    
    stream1 = std::move(stream2);
    
    EXPECT_FALSE(stream1.IsEOF());
}

// ==================== Тесты на GetFileSize ====================

TEST_F(StreamBasicTest, GetFileSize_EmptyFile) {
    auto path = CreateTestFile("empty.txt", "");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    EXPECT_EQ(stream.GetFileSize(), 0);
}

TEST_F(StreamBasicTest, GetFileSize_NonEmptyFile) {
    std::string content = "Hello, World!";
    auto path = CreateTestFile("content.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    EXPECT_EQ(stream.GetFileSize(), content.size());
}

TEST_F(StreamBasicTest, GetFileSize_LargeFile) {
    std::string content(10000, 'A');
    auto path = CreateTestFile("large.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    EXPECT_EQ(stream.GetFileSize(), 10000);
}

// ==================== Тесты на SetCursor и IsEOF ====================

TEST_F(StreamBasicTest, SetCursor_ValidPosition) {
    auto path = CreateTestFile("cursor_test.txt", "0123456789");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    EXPECT_NO_THROW({
        stream.SetCursor(5);
    });
}

TEST_F(StreamBasicTest, IsEOF_InitiallyFalse) {
    auto path = CreateTestFile("eof_test.txt", "test");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    EXPECT_FALSE(stream.IsEOF());
}

TEST_F(StreamBasicTest, IsEOF_AfterReadingAll) {
    auto path = CreateTestFile("eof_test2.txt", "test");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(100);
    stream.CRead(buffer);
    
    EXPECT_TRUE(stream.IsEOF());
}

// ==================== Тесты на Close ====================

TEST_F(StreamBasicTest, Close_Success) {
    auto path = CreateTestFile("close_test.txt", "test");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    EXPECT_NO_THROW({
        stream.Close();
    });
}

TEST_F(StreamBasicTest, Close_MultipleCalls) {
    auto path = CreateTestFile("close_test2.txt", "test");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    stream.Close();
    // Повторный вызов не должен крашить программу
    EXPECT_NO_THROW({
        stream.Close();
    });
}