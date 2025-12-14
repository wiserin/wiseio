// tests/test_string_buffer.cpp
#include <gtest/gtest.h>
#include <string>
#include <stdexcept>
#include "wise-io/buffer.hpp"
#include "wise-io/schemas.hpp"

class StringBufferTest : public ::testing::Test {
protected:
    wiseio::StringIOBuffer buffer_;
};

// ==================== ResizeBuffer ====================

TEST_F(StringBufferTest, ResizeBuffer_Increase) {
    buffer_.ResizeBuffer(10);
    EXPECT_EQ(buffer_.GetBufferSize(), 10);
}

TEST_F(StringBufferTest, ResizeBuffer_Decrease) {
    buffer_.ResizeBuffer(20);
    buffer_.ResizeBuffer(10);
    EXPECT_EQ(buffer_.GetBufferSize(), 10);
}

TEST_F(StringBufferTest, ResizeBuffer_ToZero) {
    buffer_.ResizeBuffer(10);
    buffer_.ResizeBuffer(0);
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

// ==================== SetCursor ====================

TEST_F(StringBufferTest, SetCursor_ValidPosition) {
    buffer_.ResizeBuffer(10);
    EXPECT_NO_THROW(buffer_.SetCursor(5));
}

TEST_F(StringBufferTest, SetCursor_AtBeginning) {
    buffer_.ResizeBuffer(10);
    EXPECT_NO_THROW(buffer_.SetCursor(0));
}

TEST_F(StringBufferTest, SetCursor_AtEnd) {
    buffer_.ResizeBuffer(10);
    EXPECT_NO_THROW(buffer_.SetCursor(10));
}

TEST_F(StringBufferTest, SetCursor_BeyondEnd_Throws) {
    buffer_.ResizeBuffer(10);
    EXPECT_THROW(buffer_.SetCursor(11), std::out_of_range);
}

// ==================== SetEncoding ====================

TEST_F(StringBufferTest, SetEncoding_UTF8) {
    EXPECT_NO_THROW(buffer_.SetEncoding(wiseio::Encoding::kUTF_8));
}

TEST_F(StringBufferTest, SetEncoding_UTF16) {
    EXPECT_NO_THROW(buffer_.SetEncoding(wiseio::Encoding::kUTF_16));
}

// ==================== GetLen ====================

TEST_F(StringBufferTest, GetLen_EmptyBuffer_UTF8) {
    buffer_.SetEncoding(wiseio::Encoding::kUTF_8);
    EXPECT_EQ(buffer_.GetLen(), 0);
}

TEST_F(StringBufferTest, GetLen_WithData_UTF8) {
    buffer_.SetEncoding(wiseio::Encoding::kUTF_8);
    buffer_.AddDataToBuffer("Hello");
    
    // UTF-8: 1 байт на символ для ASCII
    EXPECT_EQ(buffer_.GetLen(), 5);
}

TEST_F(StringBufferTest, GetLen_WithData_UTF16) {
    buffer_.SetEncoding(wiseio::Encoding::kUTF_16);
    buffer_.AddDataToBuffer("Hello");
    
    // UTF-16: 2 байта на символ
    // "Hello" = 5 символов * 1 байт = 5 байт
    // GetLen = 5 / 2 = 2
    EXPECT_EQ(buffer_.GetLen(), 2);
}

// ==================== AddDataToBuffer ====================

TEST_F(StringBufferTest, AddDataToBuffer_EmptyString) {
    buffer_.AddDataToBuffer("");
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

TEST_F(StringBufferTest, AddDataToBuffer_SimpleString) {
    buffer_.AddDataToBuffer("Hello");
    EXPECT_EQ(buffer_.GetBufferSize(), 5);
}

TEST_F(StringBufferTest, AddDataToBuffer_MultipleAdds) {
    buffer_.AddDataToBuffer("Hello");
    buffer_.AddDataToBuffer(" ");
    buffer_.AddDataToBuffer("World");
    
    EXPECT_EQ(buffer_.GetBufferSize(), 11);
}

TEST_F(StringBufferTest, AddDataToBuffer_WithNewlines) {
    buffer_.AddDataToBuffer("Line1\nLine2\nLine3");
    EXPECT_EQ(buffer_.GetBufferSize(), 17);
}

TEST_F(StringBufferTest, AddDataToBuffer_LargeString) {
    std::string large(10000, 'X');
    buffer_.AddDataToBuffer(large);
    EXPECT_EQ(buffer_.GetBufferSize(), 10000);
}

// ==================== IsLines ====================

TEST_F(StringBufferTest, IsLines_EmptyBuffer) {
    EXPECT_FALSE(buffer_.IsLines());
}

TEST_F(StringBufferTest, IsLines_WithData_CursorAtStart) {
    buffer_.AddDataToBuffer("Test");
    buffer_.SetCursor(0);
    EXPECT_TRUE(buffer_.IsLines());
}

TEST_F(StringBufferTest, IsLines_WithData_CursorInMiddle) {
    buffer_.AddDataToBuffer("Hello World");
    buffer_.SetCursor(5);
    EXPECT_TRUE(buffer_.IsLines());
}

TEST_F(StringBufferTest, IsLines_WithData_CursorAtEnd) {
    buffer_.AddDataToBuffer("Test");
    buffer_.SetCursor(4);
    EXPECT_FALSE(buffer_.IsLines());
}

// ==================== GetLine ====================

TEST_F(StringBufferTest, GetLine_SingleLine_NoNewline) {
    buffer_.AddDataToBuffer("Hello World");
    buffer_.SetCursor(0);
    
    std::string line = buffer_.GetLine();
    EXPECT_EQ(line, "Hello World");
}

TEST_F(StringBufferTest, GetLine_SingleLine_WithNewline) {
    buffer_.AddDataToBuffer("Hello World\n");
    buffer_.SetCursor(0);
    
    std::string line = buffer_.GetLine();
    EXPECT_EQ(line, "Hello World");
}

TEST_F(StringBufferTest, GetLine_MultipleLines) {
    buffer_.AddDataToBuffer("Line1\nLine2\nLine3\n");
    buffer_.SetCursor(0);
    
    std::string line1 = buffer_.GetLine();
    std::string line2 = buffer_.GetLine();
    std::string line3 = buffer_.GetLine();
    
    EXPECT_EQ(line1, "Line1");
    EXPECT_EQ(line2, "Line2");
    EXPECT_EQ(line3, "Line3");
}

TEST_F(StringBufferTest, GetLine_EmptyLines) {
    buffer_.AddDataToBuffer("\n\n\n");
    buffer_.SetCursor(0);
    
    std::string line1 = buffer_.GetLine();
    std::string line2 = buffer_.GetLine();
    
    EXPECT_TRUE(line1.empty() || line1 == "\n");
    EXPECT_TRUE(line2.empty() || line2 == "\n");
}

TEST_F(StringBufferTest, GetLine_NoMoreLines) {
    buffer_.AddDataToBuffer("Only one line");
    buffer_.SetCursor(0);
    
    buffer_.GetLine();
    std::string line2 = buffer_.GetLine();
    
    EXPECT_TRUE(line2.empty());
}

// ==================== SetIgnoreComments ====================

TEST_F(StringBufferTest, IgnoreComments_Disabled_Default) {
    buffer_.AddDataToBuffer("# This is a comment\nReal line\n");
    buffer_.SetCursor(0);
    
    std::string line1 = buffer_.GetLine();
    std::string line2 = buffer_.GetLine();
    
    // По умолчанию комментарии не игнорируются
    EXPECT_FALSE(line1.empty());
}

TEST_F(StringBufferTest, IgnoreComments_Enabled) {
    buffer_.SetIgnoreComments(true);
    buffer_.AddDataToBuffer("# Comment\nReal line\n");
    buffer_.SetCursor(0);
    
    std::string line = buffer_.GetLine();
    
    EXPECT_EQ(line, "Real line");
}

TEST_F(StringBufferTest, IgnoreComments_InlineComment) {
    buffer_.SetIgnoreComments(true);
    buffer_.AddDataToBuffer("Code here # inline comment\n");
    buffer_.SetCursor(0);
    
    std::string line = buffer_.GetLine();
    
    // Строка должна обрезаться до комментария
    EXPECT_EQ(line, "Code here ");
}

TEST_F(StringBufferTest, IgnoreComments_NoSpaceBeforeHash) {
    buffer_.SetIgnoreComments(true);
    buffer_.AddDataToBuffer("No#Comment\n");
    buffer_.SetCursor(0);
    
    std::string line = buffer_.GetLine();
    
    // # не в начале и без пробела перед ним - не комментарий
    EXPECT_EQ(line, "No#Comment");
}

TEST_F(StringBufferTest, IgnoreComments_MultipleComments) {
    buffer_.SetIgnoreComments(true);
    buffer_.AddDataToBuffer("# Comment 1\n# Comment 2\nReal\n# Comment 3\n");
    buffer_.SetCursor(0);
    
    std::string line = buffer_.GetLine();
    
    EXPECT_EQ(line, "Real");
}

// ==================== SetIgnoreBlank ====================

TEST_F(StringBufferTest, IgnoreBlank_Disabled) {
    buffer_.AddDataToBuffer("Line1\n\nLine2\n");
    buffer_.SetCursor(0);
    
    std::string line1 = buffer_.GetLine();
    std::string line2 = buffer_.GetLine();
    std::string line3 = buffer_.GetLine();
    
    EXPECT_EQ(line1, "Line1");
    // line2 должна быть пустой
    EXPECT_EQ(line3, "Line2");
}

// Хм, у нас нет метода SetIgnoreBlank в интерфейсе, но он упоминается в приватных полях
// Пропустим эти тесты, так как API не предоставлен

// ==================== Clear ====================

TEST_F(StringBufferTest, Clear_EmptyBuffer) {
    EXPECT_NO_THROW(buffer_.Clear());
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

TEST_F(StringBufferTest, Clear_WithData) {
    buffer_.AddDataToBuffer("Some data");
    buffer_.Clear();
    
    EXPECT_EQ(buffer_.GetBufferSize(), 0);
}

TEST_F(StringBufferTest, Clear_ResetsCursor) {
    buffer_.AddDataToBuffer("Data");
    buffer_.SetCursor(2);
    buffer_.Clear();
    
    EXPECT_NO_THROW(buffer_.SetCursor(0));
}

TEST_F(StringBufferTest, Clear_CanAddDataAfter) {
    buffer_.AddDataToBuffer("First");
    buffer_.Clear();
    buffer_.AddDataToBuffer("Second");
    
    EXPECT_EQ(buffer_.GetBufferSize(), 6);
}

// ==================== Комбинированные тесты ====================

TEST_F(StringBufferTest, Combined_MultilineWithComments) {
    buffer_.SetIgnoreComments(true);
    buffer_.AddDataToBuffer(
        "Line 1\n"
        "# Comment line\n"
        "Line 2\n"
        "Line 3 # inline\n"
        "# Another comment\n"
        "Line 4\n"
    );
    buffer_.SetCursor(0);
    
    std::string line1 = buffer_.GetLine();
    std::string line2 = buffer_.GetLine();
    std::string line3 = buffer_.GetLine();
    std::string line4 = buffer_.GetLine();
    
    EXPECT_EQ(line1, "Line 1");
    EXPECT_EQ(line2, "Line 2");
    EXPECT_EQ(line3, "Line 3 ");
    EXPECT_EQ(line4, "Line 4");
}

TEST_F(StringBufferTest, Combined_UTF16Encoding) {
    buffer_.SetEncoding(wiseio::Encoding::kUTF_16);
    buffer_.AddDataToBuffer("Test");
    
    // 4 символа = 4 байта в UTF-8
    // В UTF-16: 4/2 = 2 "символа"
    EXPECT_EQ(buffer_.GetLen(), 2);
}

TEST_F(StringBufferTest, StressTest_ManyLines) {
    std::string data;
    for (int i = 0; i < 1000; ++i) {
        data += "Line " + std::to_string(i) + "\n";
    }
    
    buffer_.AddDataToBuffer(data);
    buffer_.SetCursor(0);
    
    int count = 0;
    while (buffer_.IsLines()) {
        buffer_.GetLine();
        ++count;
    }
    
    EXPECT_EQ(count, 1000);
}

TEST_F(StringBufferTest, EdgeCase_VeryLongLine) {
    std::string long_line(100000, 'X');
    long_line += "\n";
    
    buffer_.AddDataToBuffer(long_line);
    buffer_.SetCursor(0);
    
    std::string line = buffer_.GetLine();
    EXPECT_EQ(line.size(), 100000);
}

TEST_F(StringBufferTest, EdgeCase_OnlyNewlines) {
    buffer_.AddDataToBuffer("\n\n\n\n\n");
    buffer_.SetCursor(0);
    
    int count = 0;
    while (buffer_.IsLines()) {
        buffer_.GetLine();
        ++count;
        if (count > 10) break; // Защита от бесконечного цикла
    }
    
    EXPECT_GT(count, 0);
}

TEST_F(StringBufferTest, EdgeCase_NoFinalNewline) {
    buffer_.AddDataToBuffer("Line1\nLine2\nLine3");
    buffer_.SetCursor(0);
    
    std::string line1 = buffer_.GetLine();
    std::string line2 = buffer_.GetLine();
    std::string line3 = buffer_.GetLine();
    
    EXPECT_EQ(line1, "Line1");
    EXPECT_EQ(line2, "Line2");
    EXPECT_EQ(line3, "Line3");
}

// ==================== Тесты на баги в ReadLine ====================

TEST_F(StringBufferTest, BugCheck_ReadLine_Condition) {
    buffer_.AddDataToBuffer("Test\n");
    buffer_.SetCursor(0);
    
    EXPECT_NO_THROW({
        std::string line = buffer_.GetLine();
    });
}

TEST_F(StringBufferTest, SpecialChars_Tabs) {
    buffer_.AddDataToBuffer("Col1\tCol2\tCol3\n");
    buffer_.SetCursor(0);
    
    std::string line = buffer_.GetLine();
    EXPECT_NE(line.find('\t'), std::string::npos);
}

TEST_F(StringBufferTest, SpecialChars_CarriageReturn) {
    buffer_.AddDataToBuffer("Line1\r\nLine2\r\n");
    buffer_.SetCursor(0);
    
    std::string line1 = buffer_.GetLine();
    
    // \r\n - Windows-style line ending
    EXPECT_FALSE(line1.empty());
}