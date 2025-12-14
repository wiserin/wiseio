// tests/test_integration.cpp
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <logging/logger.hpp>
#include <logging/schemas.hpp>

#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"
#include "wise-io/schemas.hpp"

namespace fs = std::filesystem;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "wiseio_integration_tests";
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

    std::string ReadFileContent(const std::string& path) {
        std::ifstream file(path);
        return std::string((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    }

    fs::path test_dir_;
};

// ==================== Stream + BytesIOBuffer ====================

TEST_F(IntegrationTest, StreamWithBytesBuffer_ReadWrite) {
    auto path = (test_dir_ / "bytes_rw.bin").string();
    
    // Записываем через BytesIOBuffer
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
        wiseio::BytesIOBuffer buffer;
        
        std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        buffer.AddDataToBuffer(data);
        
        EXPECT_TRUE(stream.CWrite(buffer));
    }
    
    // Читаем через BytesIOBuffer
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        wiseio::BytesIOBuffer buffer;
        buffer.ResizeBuffer(10);
        
        ssize_t bytes = stream.CRead(buffer);
        EXPECT_EQ(bytes, 10);
        
        buffer.SetCursor(0);
        auto read_data = buffer.ReadFromBuffer(10);
        
        std::vector<uint8_t> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        EXPECT_EQ(read_data, expected);
    }
}

TEST_F(IntegrationTest, StreamWithBytesBuffer_Chunked) {
    auto path = (test_dir_ / "bytes_chunked.bin").string();
    
    // Записываем по частям
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
        
        for (int i = 0; i < 10; ++i) {
            wiseio::BytesIOBuffer buffer;
            std::vector<uint8_t> chunk(100, i);
            buffer.AddDataToBuffer(chunk);
            stream.CWrite(buffer);
        }
    }
    
    // Читаем по частям
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        
        for (int i = 0; i < 10; ++i) {
            wiseio::BytesIOBuffer buffer;
            buffer.ResizeBuffer(100);
            
            ssize_t bytes = stream.CRead(buffer);
            EXPECT_EQ(bytes, 100);
            
            const uint8_t* ptr = buffer.GetDataPtr();
            for (size_t j = 0; j < 100; ++j) {
                EXPECT_EQ(ptr[j], i);
            }
        }
    }
}

// ==================== Stream + StringIOBuffer ====================

TEST_F(IntegrationTest, StreamWithStringBuffer_TextFile) {
    auto path = (test_dir_ / "text_file.txt").string();
    
    // Записываем текст
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
        wiseio::StringIOBuffer buffer;
        
        buffer.AddDataToBuffer("Line 1\nLine 2\nLine 3\n");
        stream.CWrite(buffer);
    }
    
    // Читаем построчно
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        wiseio::StringIOBuffer buffer;
        
        size_t file_size = stream.GetFileSize();
        buffer.ResizeBuffer(file_size);
        stream.CRead(buffer);
        
        buffer.SetCursor(0);
        
        std::string line1 = buffer.GetLine();
        std::string line2 = buffer.GetLine();
        std::string line3 = buffer.GetLine();
        
        EXPECT_EQ(line1, "Line 1");
        EXPECT_EQ(line2, "Line 2");
        EXPECT_EQ(line3, "Line 3");
    }
}

TEST_F(IntegrationTest, StreamWithStringBuffer_CommentsFilter) {
    auto path = (test_dir_ / "config.txt").string();
    
    // Записываем файл с комментариями
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
        wiseio::StringIOBuffer buffer;
        
        buffer.AddDataToBuffer(
            "# Configuration file\n"
            "setting1=value1\n"
            "# This is a comment\n"
            "setting2=value2\n"
            "setting3=value3 # inline comment\n"
        );
        
        stream.CWrite(buffer);
    }
    
    // Читаем с фильтрацией комментариев
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        wiseio::StringIOBuffer buffer;
        
        buffer.SetIgnoreComments(true);
        
        size_t file_size = stream.GetFileSize();
        buffer.ResizeBuffer(file_size);
        stream.CRead(buffer);
        
        buffer.SetCursor(0);
        
        std::vector<std::string> lines;
        while (buffer.IsLines()) {
            std::string line = buffer.GetLine();
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        
        EXPECT_GE(lines.size(), 3);
        EXPECT_EQ(lines[0], "setting1=value1");
        EXPECT_EQ(lines[1], "setting2=value2");
        EXPECT_EQ(lines[2], "setting3=value3 ");
    }
}

// ==================== Копирование файла ====================

TEST_F(IntegrationTest, FileCopy_SmallFile) {
    auto source = CreateTestFile("source.txt", "Hello, World!");
    auto dest = (test_dir_ / "dest.txt").string();
    
    // Копируем
    {
        auto read_stream = wiseio::CreateStream(source.c_str(), wiseio::OpenMode::kRead);
        auto write_stream = wiseio::CreateStream(dest.c_str(), wiseio::OpenMode::kWrite);
        
        std::vector<uint8_t> buffer(1024);
        ssize_t bytes = read_stream.CRead(buffer);
        write_stream.CWrite(buffer);
    }
    
    EXPECT_EQ(ReadFileContent(source), ReadFileContent(dest));
}

TEST_F(IntegrationTest, FileCopy_LargeFile) {
    std::string content(100000, 'X');
    auto source = CreateTestFile("large_source.bin", content);
    auto dest = (test_dir_ / "large_dest.bin").string();
    
    // Копируем блоками
    {
        auto read_stream = wiseio::CreateStream(source.c_str(), wiseio::OpenMode::kRead);
        auto write_stream = wiseio::CreateStream(dest.c_str(), wiseio::OpenMode::kWrite);
        
        while (!read_stream.IsEOF()) {
            std::vector<uint8_t> buffer(4096);
            ssize_t bytes = read_stream.CRead(buffer);
            if (bytes > 0) {
                write_stream.CWrite(buffer);
            }
        }
    }
    
    EXPECT_EQ(ReadFileContent(source), ReadFileContent(dest));
}

// ==================== Append mode ====================

TEST_F(IntegrationTest, AppendMode_MultipleWrites) {
    auto path = (test_dir_ / "append.log").string();
    
    // Первая запись
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
        stream.AWrite("Entry 1\n");
    }
    
    // Вторая запись
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
        stream.AWrite("Entry 2\n");
    }
    
    // Третья запись
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
        stream.AWrite("Entry 3\n");
    }
    
    EXPECT_EQ(ReadFileContent(path), "Entry 1\nEntry 2\nEntry 3\n");
}

TEST_F(IntegrationTest, AppendMode_WithBuffer) {
    auto path = (test_dir_ / "append_buf.log").string();
    
    for (int i = 1; i <= 5; ++i) {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kAppend);
        wiseio::BytesIOBuffer buffer;
        
        std::string entry = "Entry " + std::to_string(i) + "\n";
        std::vector<uint8_t> data(entry.begin(), entry.end());
        buffer.AddDataToBuffer(data);
        
        stream.AWrite(buffer);
    }
    
    auto content = ReadFileContent(path);
    EXPECT_NE(content.find("Entry 1"), std::string::npos);
    EXPECT_NE(content.find("Entry 5"), std::string::npos);
}

// ==================== ReadAndWrite mode ====================

TEST_F(IntegrationTest, ReadAndWrite_ModifyFile) {
    auto path = CreateTestFile("modify.txt", "0123456789");
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);
    
    // Читаем первые 5 байт
    std::vector<uint8_t> read_buf(5);
    stream.CRead(read_buf);
    EXPECT_EQ(std::string(read_buf.begin(), read_buf.end()), "01234");
    
    // Записываем в текущую позицию
    std::vector<uint8_t> write_data = {'X', 'Y', 'Z'};
    stream.CWrite(write_data);
    
    // Читаем оставшееся
    std::vector<uint8_t> rest(2);
    stream.CRead(rest);
    EXPECT_EQ(std::string(rest.begin(), rest.end()), "89");
    
    stream.Close();
    
    EXPECT_EQ(ReadFileContent(path), "01234XYZ89");
}

TEST_F(IntegrationTest, ReadAndWrite_RandomAccess) {
    auto path = (test_dir_ / "random.bin").string();
    
    // Создаём файл
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
        std::vector<uint8_t> data(100, 0);
        stream.CWrite(data);
    }
    
    // Записываем в разные позиции
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);
        
        std::vector<uint8_t> pattern1 = {1, 1, 1};
        stream.CustomWrite(pattern1, 10);
        
        std::vector<uint8_t> pattern2 = {2, 2, 2};
        stream.CustomWrite(pattern2, 50);
        
        std::vector<uint8_t> pattern3 = {3, 3, 3};
        stream.CustomWrite(pattern3, 90);
    }
    
    // Проверяем чтение из разных позиций
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        
        std::vector<uint8_t> check1(3);
        stream.CustomRead(check1, 10);
        EXPECT_EQ(check1, std::vector<uint8_t>({1, 1, 1}));
        
        std::vector<uint8_t> check2(3);
        stream.CustomRead(check2, 50);
        EXPECT_EQ(check2, std::vector<uint8_t>({2, 2, 2}));
        
        std::vector<uint8_t> check3(3);
        stream.CustomRead(check3, 90);
        EXPECT_EQ(check3, std::vector<uint8_t>({3, 3, 3}));
    }
}

// ==================== Сложный сценарий ====================

TEST_F(IntegrationTest, ComplexScenario_DataProcessing) {
    auto input_path = CreateTestFile("input.csv", 
        "# CSV File\n"
        "Name,Age,City\n"
        "Alice,30,NYC\n"
        "# Comment line\n"
        "Bob,25,LA\n"
        "Charlie,35,SF\n"
    );
    
    auto output_path = (test_dir_ / "output.csv").string();
    
    // Обработка: читаем с фильтрацией, пишем в новый файл
    {
        auto read_stream = wiseio::CreateStream(input_path.c_str(), wiseio::OpenMode::kRead);
        auto write_stream = wiseio::CreateStream(output_path.c_str(), wiseio::OpenMode::kWrite);
        
        wiseio::StringIOBuffer read_buffer;
        read_buffer.SetIgnoreComments(true);
        
        size_t file_size = read_stream.GetFileSize();
        read_buffer.ResizeBuffer(file_size);
        read_stream.CRead(read_buffer);
        
        read_buffer.SetCursor(0);
        
        wiseio::StringIOBuffer write_buffer;
        
        while (read_buffer.IsLines()) {
            std::string line = read_buffer.GetLine();
            if (!line.empty()) {
                write_buffer.AddDataToBuffer(line + "\n");
            }
        }
        
        write_stream.CWrite(write_buffer);
    }
    
    // Проверяем результат
    std::string result = ReadFileContent(output_path);
    EXPECT_EQ(result.find("# CSV File"), std::string::npos);
    EXPECT_EQ(result.find("# Comment line"), std::string::npos);
    EXPECT_NE(result.find("Alice"), std::string::npos);
    EXPECT_NE(result.find("Bob"), std::string::npos);
}

TEST_F(IntegrationTest, ComplexScenario_BinaryProtocol) {
    auto path = (test_dir_ / "protocol.bin").string();
    
    // Записываем бинарный протокол: [length:4][data:length]
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
        
        std::vector<uint8_t> message1 = {'H', 'e', 'l', 'l', 'o'};
        uint32_t len1 = message1.size();
        
        wiseio::BytesIOBuffer buffer;
        buffer.AddDataToBuffer({
            static_cast<uint8_t>(len1 & 0xFF),
            static_cast<uint8_t>((len1 >> 8) & 0xFF),
            static_cast<uint8_t>((len1 >> 16) & 0xFF),
            static_cast<uint8_t>((len1 >> 24) & 0xFF)
        });
        buffer.AddDataToBuffer(message1);
        
        stream.CWrite(buffer);
    }
    
    // Читаем протокол
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        
        wiseio::BytesIOBuffer len_buffer;
        len_buffer.ResizeBuffer(4);
        stream.CRead(len_buffer);
        
        const uint8_t* len_ptr = len_buffer.GetDataPtr();
        uint32_t length = len_ptr[0] | (len_ptr[1] << 8) | (len_ptr[2] << 16) | (len_ptr[3] << 24);
        
        EXPECT_EQ(length, 5);
        
        wiseio::BytesIOBuffer msg_buffer;
        msg_buffer.ResizeBuffer(length);
        stream.CRead(msg_buffer);
        
        msg_buffer.SetCursor(0);
        auto data = msg_buffer.ReadFromBuffer(length);
        
        std::string message(data.begin(), data.end());
        EXPECT_EQ(message, "Hello");
    }
}

// ==================== Стресс-тест ====================

TEST_F(IntegrationTest, StressTest_ManySmallWrites) {
    auto path = (test_dir_ / "stress_writes.log").string();
    
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
    
    for (int i = 0; i < 1000; ++i) {
        std::string line = "Line " + std::to_string(i) + "\n";
        std::vector<uint8_t> data(line.begin(), line.end());
        stream.CWrite(data);
    }
    
    stream.Close();
    
    size_t file_size = fs::file_size(path);
    EXPECT_GT(file_size, 6000); // Примерно 1000 * 7 символов минимум
}

TEST_F(IntegrationTest, StressTest_LargeBuffer) {
    auto path = (test_dir_ / "stress_large.bin").string();
    
    {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kWrite);
        wiseio::BytesIOBuffer buffer;
        
        // Добавляем 10 МБ данных
        for (int i = 0; i < 10000; ++i) {
            std::vector<uint8_t> chunk(1000, i % 256);
            buffer.AddDataToBuffer(chunk);
        }
        
        stream.CWrite(buffer);
    }
    
    size_t file_size = fs::file_size(path);
    EXPECT_EQ(file_size, 10000000);
}