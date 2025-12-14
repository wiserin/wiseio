// tests/test_stream_read.cpp
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"
#include "wise-io/schemas.hpp"

namespace fs = std::filesystem;

class StreamReadTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "wiseio_read_tests";
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
        std::ofstream file(path, std::ios::binary);
        file << content;
        file.close();
        return path.string();
    }

    std::string CreateBinaryFile(const std::string& name, const std::vector<uint8_t>& data) {
        auto path = test_dir_ / name;
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
        return path.string();
    }

    fs::path test_dir_;
};

// ==================== CRead с vector<uint8_t> ====================

TEST_F(StreamReadTest, CRead_Vector_EmptyFile) {
    auto path = CreateTestFile("empty.bin", "");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(10);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, 0);
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_TRUE(stream.IsEOF());
}

TEST_F(StreamReadTest, CRead_Vector_SmallFile) {
    std::string content = "Hello";
    auto path = CreateTestFile("small.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(10);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, content.size());
    EXPECT_EQ(buffer.size(), content.size());
    EXPECT_TRUE(std::equal(buffer.begin(), buffer.end(), content.begin()));
}

TEST_F(StreamReadTest, CRead_Vector_ExactSize) {
    std::string content = "12345";
    auto path = CreateTestFile("exact.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(5);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, 5);
    EXPECT_EQ(buffer.size(), 5);
}

TEST_F(StreamReadTest, CRead_Vector_MultipleReads) {
    std::string content = "0123456789";
    auto path = CreateTestFile("multiple.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer1(5);
    ssize_t bytes1 = stream.CRead(buffer1);
    
    std::vector<uint8_t> buffer2(5);
    ssize_t bytes2 = stream.CRead(buffer2);
    
    EXPECT_EQ(bytes1, 5);
    EXPECT_EQ(bytes2, 5);
    EXPECT_EQ(std::string(buffer1.begin(), buffer1.end()), "01234");
    EXPECT_EQ(std::string(buffer2.begin(), buffer2.end()), "56789");
}

TEST_F(StreamReadTest, CRead_Vector_LargeFile) {
    std::string content(10000, 'X');
    auto path = CreateTestFile("large.bin", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(10000);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, 10000);
    EXPECT_EQ(buffer.size(), 10000);
}

TEST_F(StreamReadTest, CRead_Vector_AfterEOF) {
    auto path = CreateTestFile("eof.txt", "test");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer1(10);
    stream.CRead(buffer1);
    
    std::vector<uint8_t> buffer2(10);
    ssize_t bytes_read = stream.CRead(buffer2);
    
    EXPECT_EQ(bytes_read, 0);
}

// ==================== CRead с IOBuffer ====================

TEST_F(StreamReadTest, CRead_BytesBuffer_Success) {
    std::string content = "Buffer Test";
    auto path = CreateTestFile("buffer.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    wiseio::BytesIOBuffer buffer;
    buffer.ResizeBuffer(20);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, content.size());
    EXPECT_EQ(buffer.GetBufferSize(), content.size());
}

TEST_F(StreamReadTest, CRead_StringBuffer_Success) {
    std::string content = "String Buffer Test";
    auto path = CreateTestFile("string_buf.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    wiseio::StringIOBuffer buffer;
    buffer.ResizeBuffer(30);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, content.size());
}

// ==================== CRead с std::string ====================

TEST_F(StreamReadTest, CRead_String_Success) {
    std::string content = "String Read Test";
    auto path = CreateTestFile("str_read.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::string buffer(20, '\0');
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, content.size());
    EXPECT_EQ(buffer.size(), content.size());
    EXPECT_EQ(buffer, content);
}

TEST_F(StreamReadTest, CRead_String_EmptyFile) {
    auto path = CreateTestFile("empty_str.txt", "");
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::string buffer(10, '\0');
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, 0);
    EXPECT_EQ(buffer.size(), 0);
}

// ==================== CustomRead с offset ====================

TEST_F(StreamReadTest, CustomRead_Vector_WithOffset) {
    std::string content = "0123456789";
    auto path = CreateTestFile("offset.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(5);
    ssize_t bytes_read = stream.CustomRead(buffer, 3);
    
    EXPECT_EQ(bytes_read, 5);
    EXPECT_EQ(std::string(buffer.begin(), buffer.end()), "34567");
}

TEST_F(StreamReadTest, CustomRead_Vector_OffsetZero) {
    std::string content = "ABCDE";
    auto path = CreateTestFile("offset_zero.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(3);
    ssize_t bytes_read = stream.CustomRead(buffer, 0);
    
    EXPECT_EQ(bytes_read, 3);
    EXPECT_EQ(std::string(buffer.begin(), buffer.end()), "ABC");
}

TEST_F(StreamReadTest, CustomRead_Vector_OffsetAtEnd) {
    std::string content = "12345";
    auto path = CreateTestFile("offset_end.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(5);
    ssize_t bytes_read = stream.CustomRead(buffer, 5);
    
    EXPECT_EQ(bytes_read, 0);
    EXPECT_TRUE(stream.IsEOF());
}

TEST_F(StreamReadTest, CustomRead_Vector_OffsetBeyondEnd) {
    std::string content = "123";
    auto path = CreateTestFile("offset_beyond.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(5);
    ssize_t bytes_read = stream.CustomRead(buffer, 10);
    
    EXPECT_EQ(bytes_read, 0);
}

TEST_F(StreamReadTest, CustomRead_String_WithOffset) {
    std::string content = "Hello, World!";
    auto path = CreateTestFile("str_offset.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::string buffer(5, '\0');
    ssize_t bytes_read = stream.CustomRead(buffer, 7);
    
    EXPECT_EQ(bytes_read, 5);
    EXPECT_EQ(buffer, "World");
}

TEST_F(StreamReadTest, CustomRead_Buffer_WithOffset) {
    std::string content = "CustomBufferTest";
    auto path = CreateTestFile("buf_offset.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    wiseio::BytesIOBuffer buffer;
    buffer.ResizeBuffer(6);
    ssize_t bytes_read = stream.CustomRead(buffer, 6);
    
    EXPECT_EQ(bytes_read, 6);
}

// ==================== Тесты на бинарные данные ====================

TEST_F(StreamReadTest, CRead_BinaryData) {
    std::vector<uint8_t> data = {0x00, 0xFF, 0x01, 0xFE, 0x7F, 0x80};
    auto path = CreateBinaryFile("binary.bin", data);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(6);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, 6);
    EXPECT_EQ(buffer, data);
}

TEST_F(StreamReadTest, CRead_BinaryData_WithNulls) {
    std::vector<uint8_t> data = {0x01, 0x00, 0x00, 0x02, 0x00, 0x03};
    auto path = CreateBinaryFile("nulls.bin", data);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    std::vector<uint8_t> buffer(6);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, 6);
    EXPECT_EQ(buffer, data);
}

// ==================== Тесты на SetCursor с чтением ====================

TEST_F(StreamReadTest, SetCursor_ThenRead) {
    std::string content = "0123456789";
    auto path = CreateTestFile("cursor_read.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    stream.SetCursor(5);
    
    std::vector<uint8_t> buffer(5);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, 5);
    EXPECT_EQ(std::string(buffer.begin(), buffer.end()), "56789");
}

TEST_F(StreamReadTest, MultipleSetCursor_Reads) {
    std::string content = "ABCDEFGHIJ";
    auto path = CreateTestFile("multi_cursor.txt", content);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
    
    stream.SetCursor(2);
    std::vector<uint8_t> buf1(2);
    stream.CRead(buf1);
    EXPECT_EQ(std::string(buf1.begin(), buf1.end()), "CD");
    
    stream.SetCursor(7);
    std::vector<uint8_t> buf2(2);
    stream.CRead(buf2);
    EXPECT_EQ(std::string(buf2.begin(), buf2.end()), "HI");
}