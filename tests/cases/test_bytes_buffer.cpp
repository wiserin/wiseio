// tests/test_bytes_buffer.cpp
#include <gtest/gtest.h>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "wise-io/buffer.hpp"

class BytesBufferTest : public ::testing::Test {
protected:
    wiseio::BytesIOBuffer buffer_;
};

// ==================== ResizeBuffer ====================

TEST_F(BytesBufferTest, ResizeBuffer_Increase) {
    buffer_.ResizeBuffer(10);
    EXPECT_EQ(buffer_.GetBufferSize(), 10);
}

TEST_F(BytesBufferTest, ResizeBuffer_Decrease) {
    buffer_.ResizeBuffer(20);
    buffer_.ResizeBuffer(10);
    EXPECT_EQ(buffer_.GetBufferSize(), 10);
}

TEST_F(BytesBufferTest, ResizeBuffer_ToZero) {
    buffer_.ResizeBuffer(10);
    buffer_.ResizeBuffer(0);
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

TEST_F(BytesBufferTest, ResizeBuffer_Large) {
    buffer_.ResizeBuffer(100000);
    EXPECT_EQ(buffer_.GetBufferSize(), 100000);
}

// ==================== SetCursor ====================

TEST_F(BytesBufferTest, SetCursor_ValidPosition) {
    buffer_.ResizeBuffer(10);
    EXPECT_NO_THROW(buffer_.SetCursor(5));
}

TEST_F(BytesBufferTest, SetCursor_AtBeginning) {
    buffer_.ResizeBuffer(10);
    EXPECT_NO_THROW(buffer_.SetCursor(0));
}

TEST_F(BytesBufferTest, SetCursor_AtEnd) {
    buffer_.ResizeBuffer(10);
    EXPECT_NO_THROW(buffer_.SetCursor(10));
}

TEST_F(BytesBufferTest, SetCursor_BeyondEnd_Throws) {
    buffer_.ResizeBuffer(10);
    EXPECT_THROW(buffer_.SetCursor(11), std::out_of_range);
}

TEST_F(BytesBufferTest, SetCursor_EmptyBuffer_Zero) {
    EXPECT_NO_THROW(buffer_.SetCursor(0));
}

TEST_F(BytesBufferTest, SetCursor_EmptyBuffer_NonZero_Throws) {
    EXPECT_THROW(buffer_.SetCursor(1), std::out_of_range);
}

// ==================== AddDataToBuffer ====================

TEST_F(BytesBufferTest, AddDataToBuffer_EmptyVector) {
    std::vector<uint8_t> data;
    buffer_.AddDataToBuffer(data);
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

TEST_F(BytesBufferTest, AddDataToBuffer_SmallVector) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data);
    
    EXPECT_EQ(buffer_.GetBufferSize(), 5);
    
    const uint8_t* ptr = buffer_.GetDataPtr();
    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_EQ(ptr[i], data[i]);
    }
}

TEST_F(BytesBufferTest, AddDataToBuffer_MultipleAdds) {
    std::vector<uint8_t> data1 = {1, 2, 3};
    std::vector<uint8_t> data2 = {4, 5, 6};
    std::vector<uint8_t> data3 = {7, 8, 9};
    
    buffer_.AddDataToBuffer(data1);
    buffer_.AddDataToBuffer(data2);
    buffer_.AddDataToBuffer(data3);
    
    EXPECT_EQ(buffer_.GetBufferSize(), 9);
    
    const uint8_t* ptr = buffer_.GetDataPtr();
    std::vector<uint8_t> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(ptr[i], expected[i]);
    }
}

TEST_F(BytesBufferTest, AddDataToBuffer_LargeData) {
    std::vector<uint8_t> data(10000, 0xFF);
    buffer_.AddDataToBuffer(data);
    
    EXPECT_EQ(buffer_.GetBufferSize(), 10000);
}

TEST_F(BytesBufferTest, AddDataToBuffer_BinaryData) {
    std::vector<uint8_t> data = {0x00, 0xFF, 0x01, 0xFE, 0x7F, 0x80};
    buffer_.AddDataToBuffer(data);
    
    const uint8_t* ptr = buffer_.GetDataPtr();
    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_EQ(ptr[i], data[i]);
    }
}

// ==================== IsData ====================

TEST_F(BytesBufferTest, IsData_EmptyBuffer) {
    EXPECT_FALSE(buffer_.IsData());
}

TEST_F(BytesBufferTest, IsData_WithData_CursorAtStart) {
    std::vector<uint8_t> data = {1, 2, 3};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(0);
    
    EXPECT_TRUE(buffer_.IsData());
}

TEST_F(BytesBufferTest, IsData_WithData_CursorInMiddle) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(2);
    
    EXPECT_TRUE(buffer_.IsData());
}

TEST_F(BytesBufferTest, IsData_WithData_CursorAtEnd) {
    std::vector<uint8_t> data = {1, 2, 3};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(3);
    
    EXPECT_FALSE(buffer_.IsData());
}

TEST_F(BytesBufferTest, IsData_AfterResize) {
    buffer_.ResizeBuffer(5);
    buffer_.SetCursor(0);
    
    EXPECT_TRUE(buffer_.IsData());
}

// ==================== ReadFromBuffer ====================

TEST_F(BytesBufferTest, ReadFromBuffer_EmptyBuffer) {
    auto result = buffer_.ReadFromBuffer(10);
    EXPECT_EQ(result.size(), 0);
}

TEST_F(BytesBufferTest, ReadFromBuffer_SmallRead) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(0);
    
    auto result = buffer_.ReadFromBuffer(3);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
}

TEST_F(BytesBufferTest, ReadFromBuffer_ExactSize) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(0);
    
    auto result = buffer_.ReadFromBuffer(5);
    
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result, data);
}

TEST_F(BytesBufferTest, ReadFromBuffer_MoreThanAvailable) {
    std::vector<uint8_t> data = {1, 2, 3};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(0);
    
    auto result = buffer_.ReadFromBuffer(10);
    
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result, data);
}

TEST_F(BytesBufferTest, ReadFromBuffer_MultipleReads) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(0);
    
    auto result1 = buffer_.ReadFromBuffer(3);
    auto result2 = buffer_.ReadFromBuffer(3);
    auto result3 = buffer_.ReadFromBuffer(3);
    
    EXPECT_EQ(result1.size(), 3);
    EXPECT_EQ(result2.size(), 3);
    EXPECT_EQ(result3.size(), 3);
    
    EXPECT_EQ(result1[0], 1);
    EXPECT_EQ(result2[0], 4);
    EXPECT_EQ(result3[0], 7);
}

TEST_F(BytesBufferTest, ReadFromBuffer_FromMiddle) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(2);
    
    auto result = buffer_.ReadFromBuffer(2);
    
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 3);
    EXPECT_EQ(result[1], 4);
}

TEST_F(BytesBufferTest, ReadFromBuffer_Zero) {
    std::vector<uint8_t> data = {1, 2, 3};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(0);
    
    auto result = buffer_.ReadFromBuffer(0);
    
    EXPECT_EQ(result.size(), 0);
}

TEST_F(BytesBufferTest, ReadFromBuffer_UpdatesCursor) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(0);
    
    buffer_.ReadFromBuffer(3);
    
    // Курсор должен переместиться на 3 позиции
    auto result = buffer_.ReadFromBuffer(1);
    EXPECT_EQ(result[0], 4);
}

// ==================== Clear ====================

TEST_F(BytesBufferTest, Clear_EmptyBuffer) {
    EXPECT_NO_THROW(buffer_.Clear());
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

TEST_F(BytesBufferTest, Clear_WithData) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data);
    
    buffer_.Clear();
    
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

TEST_F(BytesBufferTest, Clear_ResetsCursor) {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(3);
    
    buffer_.Clear();
    
    // Курсор должен быть сброшен на 0
    EXPECT_NO_THROW(buffer_.SetCursor(0));
}

TEST_F(BytesBufferTest, Clear_CanAddDataAfter) {
    std::vector<uint8_t> data1 = {1, 2, 3};
    buffer_.AddDataToBuffer(data1);
    buffer_.Clear();
    
    std::vector<uint8_t> data2 = {4, 5, 6};
    buffer_.AddDataToBuffer(data2);
    
    EXPECT_EQ(buffer_.GetBufferSize(), 3);
    
    const uint8_t* ptr = buffer_.GetDataPtr();
    for (size_t i = 0; i < data2.size(); ++i) {
        EXPECT_EQ(ptr[i], data2[i]);
    }
}

TEST_F(BytesBufferTest, Clear_MultipleTimes) {
    buffer_.Clear();
    buffer_.Clear();
    buffer_.Clear();
    
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

// ==================== Комбинированные тесты ====================

TEST_F(BytesBufferTest, Combined_AddReadClear) {
    // Добавляем данные
    std::vector<uint8_t> data1 = {1, 2, 3, 4, 5};
    buffer_.AddDataToBuffer(data1);
    
    // Читаем часть
    buffer_.SetCursor(0);
    auto result1 = buffer_.ReadFromBuffer(3);
    EXPECT_EQ(result1.size(), 3);
    
    // Очищаем
    buffer_.Clear();
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
    
    // Добавляем новые данные
    std::vector<uint8_t> data2 = {10, 20, 30};
    buffer_.AddDataToBuffer(data2);
    EXPECT_EQ(buffer_.GetBufferSize(), 3);
}

TEST_F(BytesBufferTest, Combined_ResizeAndAdd) {
    buffer_.ResizeBuffer(10);
    
    std::vector<uint8_t> data = {1, 2, 3};
    buffer_.AddDataToBuffer(data);
    
    // Размер должен быть 10 + 3 = 13
    EXPECT_EQ(buffer_.GetBufferSize(), 13);
}

TEST_F(BytesBufferTest, EdgeCase_MaxCursor) {
    buffer_.ResizeBuffer(100);
    
    // Устанавливаем курсор в конец
    EXPECT_NO_THROW(buffer_.SetCursor(100));
    
    // IsData должен вернуть false
    EXPECT_FALSE(buffer_.IsData());
    
    // ReadFromBuffer должен вернуть пустой вектор
    auto result = buffer_.ReadFromBuffer(10);
    EXPECT_EQ(result.size(), 0);
}

TEST_F(BytesBufferTest, StressTest_ManyOperations) {
    for (int i = 0; i < 1000; ++i) {
        std::vector<uint8_t> data = {static_cast<uint8_t>(i % 256)};
        buffer_.AddDataToBuffer(data);
    }
    
    EXPECT_EQ(buffer_.GetBufferSize(), 1000);
    
    buffer_.SetCursor(0);
    for (int i = 0; i < 1000; ++i) {
        auto result = buffer_.ReadFromBuffer(1);
        EXPECT_EQ(result[0], static_cast<uint8_t>(i % 256));
    }
}