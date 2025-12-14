// tests/test_stream_write.cpp
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"
#include "wise-io/schemas.hpp"

namespace fs = std::filesystem;

class StreamWriteTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "wiseio_write_tests";
        fs::create_directories(test_dir_);
        logging::Logger::SetupLogger(logging::LoggerMode::kDebug, logging::LoggerIOMode::kSync, true);
    }

    void TearDown() override {
        if (fs::exists(test_dir_)) {
            fs::remove_all(test_dir_);
        }
    }

    std::string ReadFileContent(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    }

    std::vector<uint8_t> ReadFileBinary(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
    }

    fs::path test_dir_;
};

// ==================== CWrite с vector<uint8_t> ====================

TEST_F(StreamWriteTest, CWrite_Vector_EmptyVector) {
    auto path = (test_dir_ / "empty_write.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::vector<uint8_t> data;
    bool result = stream.CWrite(data);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path).size(), 0);
}

TEST_F(StreamWriteTest, CWrite_Vector_SmallData) {
    auto path = (test_dir_ / "small_write.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::string data_str = "Hello";
    std::vector<uint8_t> data(data_str.begin(), data_str.end());
    bool result = stream.CWrite(data);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), data_str);
}

TEST_F(StreamWriteTest, CWrite_Vector_LargeData) {
    auto path = (test_dir_ / "large_write.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::string data_str(10000, 'X');
    std::vector<uint8_t> data(data_str.begin(), data_str.end());
    bool result = stream.CWrite(data);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), data_str);
}

TEST_F(StreamWriteTest, CWrite_Vector_MultipleWrites) {
    auto path = (test_dir_ / "multi_write.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::vector<uint8_t> data1 = {'A', 'B', 'C'};
    std::vector<uint8_t> data2 = {'D', 'E', 'F'};
    
    stream.CWrite(data1);
    stream.CWrite(data2);
    
    EXPECT_EQ(ReadFileContent(path), "ABCDEF");
}

TEST_F(StreamWriteTest, CWrite_Vector_BinaryData) {
    auto path = (test_dir_ / "binary_write.bin").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::vector<uint8_t> data = {0x00, 0xFF, 0x01, 0xFE, 0x7F, 0x80};
    bool result = stream.CWrite(data);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileBinary(path), data);
}

TEST_F(StreamWriteTest, CWrite_Vector_WrongMode_Throws) {
    auto path = (test_dir_ / "wrong_mode.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
    
    std::vector<uint8_t> data = {'A', 'B', 'C'};
    bool result = stream.CWrite(data);
    
    EXPECT_FALSE(result);
}

// ==================== CWrite с IOBuffer ====================

TEST_F(StreamWriteTest, CWrite_BytesBuffer_Success) {
    auto path = (test_dir_ / "buffer_write.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    wiseio::BytesIOBuffer buffer;
    std::vector<uint8_t> data = {'T', 'E', 'S', 'T'};
    buffer.AddDataToBuffer(data);
    
    bool result = stream.CWrite(buffer);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), "TEST");
}

TEST_F(StreamWriteTest, CWrite_StringBuffer_Success) {
    auto path = (test_dir_ / "str_buffer_write.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    wiseio::StringIOBuffer buffer;
    buffer.AddDataToBuffer("String Buffer");
    
    bool result = stream.CWrite(buffer);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), "String Buffer");
}

// ==================== CWrite с std::string ====================

TEST_F(StreamWriteTest, CWrite_String_Success) {
    auto path = (test_dir_ / "string_write.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::string data = "Hello, World!";
    bool result = stream.CWrite(data);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), data);
}

TEST_F(StreamWriteTest, CWrite_String_EmptyString) {
    auto path = (test_dir_ / "empty_string.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::string data = "";
    bool result = stream.CWrite(data);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), "");
}

TEST_F(StreamWriteTest, CWrite_String_WithNulls) {
    auto path = (test_dir_ / "nulls_string.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::string data = "AB\0CD\0EF";
    bool result = stream.CWrite(data);
    
    EXPECT_TRUE(result);
}

// ==================== AWrite (Append mode) ====================

TEST_F(StreamWriteTest, AWrite_Vector_NewFile) {
    auto path = (test_dir_ / "append_new.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
    
    std::vector<uint8_t> data = {'A', 'B', 'C'};
    bool result = stream.AWrite(data);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), "ABC");
}

TEST_F(StreamWriteTest, AWrite_Vector_ExistingFile) {
    auto path = (test_dir_ / "append_existing.txt").string();
    
    // Создаём файл с начальным содержимым
    {
        std::ofstream file(path);
        file << "Initial";
    }
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
    std::vector<uint8_t> data = {'A', 'B', 'C'};
    stream.AWrite(data);
    
    EXPECT_EQ(ReadFileContent(path), "InitialABC");
}

TEST_F(StreamWriteTest, AWrite_MultipleAppends) {
    auto path = (test_dir_ / "multi_append.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
    
    std::vector<uint8_t> data1 = {'1', '2', '3'};
    std::vector<uint8_t> data2 = {'4', '5', '6'};
    std::vector<uint8_t> data3 = {'7', '8', '9'};
    
    stream.AWrite(data1);
    stream.AWrite(data2);
    stream.AWrite(data3);
    
    EXPECT_EQ(ReadFileContent(path), "123456789");
}

TEST_F(StreamWriteTest, AWrite_String_Success) {
    auto path = (test_dir_ / "append_string.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
    
    stream.AWrite("Line1\n");
    stream.AWrite("Line2\n");
    
    EXPECT_EQ(ReadFileContent(path), "Line1\nLine2\n");
}

TEST_F(StreamWriteTest, AWrite_Buffer_Success) {
    auto path = (test_dir_ / "append_buffer.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
    
    wiseio::BytesIOBuffer buffer;
    buffer.AddDataToBuffer({'X', 'Y', 'Z'});
    
    bool result = stream.AWrite(buffer);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), "XYZ");
}

TEST_F(StreamWriteTest, AWrite_WrongMode_Fails) {
    auto path = (test_dir_ / "wrong_append_mode.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::vector<uint8_t> data = {'A'};
    bool result = stream.AWrite(data);
    
    EXPECT_FALSE(result);
}

// ==================== CustomWrite с offset ====================

TEST_F(StreamWriteTest, CustomWrite_Vector_WithOffset) {
    auto path = (test_dir_ / "custom_write.txt").string();
    
    // Создаём файл с начальным содержимым
    {
        std::ofstream file(path);
        file << "0123456789";
    }
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    std::vector<uint8_t> data = {'X', 'Y', 'Z'};
    bool result = stream.CustomWrite(data, 3);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), "012XYZ6789");
}

TEST_F(StreamWriteTest, CustomWrite_Vector_OffsetZero) {
    auto path = (test_dir_ / "custom_zero.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    std::vector<uint8_t> data = {'A', 'B', 'C'};
    bool result = stream.CustomWrite(data, 0);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), "ABC");
}

TEST_F(StreamWriteTest, CustomWrite_Vector_OffsetBeyondEnd) {
    auto path = (test_dir_ / "custom_beyond.txt").string();
    
    {
        std::ofstream file(path);
        file << "123";
    }
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    std::vector<uint8_t> data = {'X', 'Y'};
    stream.CustomWrite(data, 10);
    
    // Файл должен быть расширен с нулями между концом и новыми данными
    auto content = ReadFileBinary(path);
    EXPECT_GE(content.size(), 12);
}

TEST_F(StreamWriteTest, CustomWrite_String_WithOffset) {
    auto path = (test_dir_ / "custom_str.txt").string();
    
    {
        std::ofstream file(path);
        file << "AAAAAAAAAA";
    }
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    std::string data = "BBB";
    stream.CustomWrite(data, 3);
    
    EXPECT_EQ(ReadFileContent(path), "AAABBBAAAA");
}

TEST_F(StreamWriteTest, CustomWrite_Buffer_WithOffset) {
    auto path = (test_dir_ / "custom_buf.txt").string();
    
    {
        std::ofstream file(path);
        file << "12345";
    }
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    wiseio::BytesIOBuffer buffer;
    buffer.AddDataToBuffer({'X', 'Y'});
    stream.CustomWrite(buffer, 2);
    
    EXPECT_EQ(ReadFileContent(path), "12XY5");
}

TEST_F(StreamWriteTest, CustomWrite_WrongMode_Fails) {
    auto path = (test_dir_ / "wrong_custom_mode.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
    
    std::vector<uint8_t> data = {'A'};
    bool result = stream.CustomWrite(data, 0);
    
    EXPECT_FALSE(result);
}

// ==================== ReadAndWrite mode ====================

TEST_F(StreamWriteTest, ReadAndWrite_Mode_CanWrite) {
    auto path = (test_dir_ / "rw_write.txt").string();
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);
    
    std::vector<uint8_t> data = {'R', 'W'};
    bool result = stream.CWrite(data);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(ReadFileContent(path), "RW");
}

TEST_F(StreamWriteTest, ReadAndWrite_Mode_CanRead) {
    auto path = (test_dir_ / "rw_read.txt").string();
    
    {
        std::ofstream file(path);
        file << "Test";
    }
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);
    
    std::vector<uint8_t> buffer(4);
    ssize_t bytes_read = stream.CRead(buffer);
    
    EXPECT_EQ(bytes_read, 4);
    EXPECT_EQ(std::string(buffer.begin(), buffer.end()), "Test");
}

TEST_F(StreamWriteTest, ReadAndWrite_Mode_WriteAfterRead) {
    auto path = (test_dir_ / "rw_combo.txt").string();
    
    {
        std::ofstream file(path);
        file << "0123456789";
    }
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);
    
    // Читаем первые 5 байт
    std::vector<uint8_t> read_buf(5);
    stream.CRead(read_buf);
    
    // Записываем поверх следующих
    std::vector<uint8_t> write_data = {'X', 'Y', 'Z'};
    stream.CWrite(write_data);
    
    EXPECT_EQ(ReadFileContent(path), "01234XYZ89");
}

// ==================== Тесты на SetCursor с записью ====================

TEST_F(StreamWriteTest, SetCursor_ThenWrite) {
    auto path = (test_dir_ / "cursor_write.txt").string();
    
    {
        std::ofstream file(path);
        file << "0000000000";
    }
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    stream.SetCursor(3);
    
    std::vector<uint8_t> data = {'A', 'B', 'C'};
    stream.CWrite(data);
    
    EXPECT_EQ(ReadFileContent(path), "000ABC0000");
}