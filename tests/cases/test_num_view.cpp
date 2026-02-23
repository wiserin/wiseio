// NOLINTBEGIN
#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include <vector>

#include "wise-io/byte/views.hpp"
#include "wise-io/schemas.hpp"

// ==================== GetNum — Little Endian ====================

TEST(NumViewTest, GetNum_Uint8_LE) {
    std::vector<uint8_t> data = {0xAB};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint8_t>(), 0xAB);
}

TEST(NumViewTest, GetNum_Uint16_LE) {
    std::vector<uint8_t> data = {0x01, 0x02};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint16_t>(), 0x0201u);
}

TEST(NumViewTest, GetNum_Uint32_LE_Zero) {
    std::vector<uint8_t> data = {0x00, 0x00, 0x00, 0x00};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 0u);
}

TEST(NumViewTest, GetNum_Uint32_LE_MaxValue) {
    std::vector<uint8_t> data = {0xFF, 0xFF, 0xFF, 0xFF};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), std::numeric_limits<uint32_t>::max());
}

TEST(NumViewTest, GetNum_Uint32_LE_KnownValue) {
    std::vector<uint8_t> data = {0x78, 0x56, 0x34, 0x12};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 0x12345678u);
}

TEST(NumViewTest, GetNum_Uint32_LE_Value250) {
    std::vector<uint8_t> data = {0xFA, 0x00, 0x00, 0x00};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 250u);
}

TEST(NumViewTest, GetNum_Uint64_LE) {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint64_t>(), 0x0807060504030201ULL);
}

// ==================== GetNum — Big Endian ====================

TEST(NumViewTest, GetNum_Uint16_BE) {
    std::vector<uint8_t> data = {0x01, 0x02};
    wiseio::NumView view(data, wiseio::Endianness::kBigEndian);
    EXPECT_EQ(view.GetNum<uint16_t>(), 0x0102u);
}

TEST(NumViewTest, GetNum_Uint32_BE_KnownValue) {
    std::vector<uint8_t> data = {0x12, 0x34, 0x56, 0x78};
    wiseio::NumView view(data, wiseio::Endianness::kBigEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 0x12345678u);
}

TEST(NumViewTest, GetNum_LE_vs_BE_ProduceDifferentResults) {
    std::vector<uint8_t> data1 = {0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> data2 = {0x01, 0x02, 0x03, 0x04};

    wiseio::NumView le(data1, wiseio::Endianness::kLittleEndian);
    wiseio::NumView be(data2, wiseio::Endianness::kBigEndian);

    EXPECT_NE(le.GetNum<uint32_t>(), be.GetNum<uint32_t>());
    EXPECT_EQ(le.GetNum<uint32_t>(), 0x04030201u);
    EXPECT_EQ(be.GetNum<uint32_t>(), 0x01020304u);
}

// ==================== SetNum ====================

TEST(NumViewTest, SetNum_Uint32_LE) {
    std::vector<uint8_t> data(4, 0);
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    view.SetNum<uint32_t>(0x12345678);

    EXPECT_EQ(data[0], 0x78);
    EXPECT_EQ(data[1], 0x56);
    EXPECT_EQ(data[2], 0x34);
    EXPECT_EQ(data[3], 0x12);
}

TEST(NumViewTest, SetNum_Uint32_BE) {
    std::vector<uint8_t> data(4, 0);
    wiseio::NumView view(data, wiseio::Endianness::kBigEndian);
    view.SetNum<uint32_t>(0x12345678);

    EXPECT_EQ(data[0], 0x12);
    EXPECT_EQ(data[1], 0x34);
    EXPECT_EQ(data[2], 0x56);
    EXPECT_EQ(data[3], 0x78);
}

TEST(NumViewTest, SetNum_Uint32_LE_Zero) {
    std::vector<uint8_t> data = {0xFF, 0xFF, 0xFF, 0xFF};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    view.SetNum<uint32_t>(0);

    for (auto b : data) EXPECT_EQ(b, 0x00);
}

TEST(NumViewTest, SetNum_Uint32_LE_Max) {
    std::vector<uint8_t> data(4, 0);
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    view.SetNum<uint32_t>(std::numeric_limits<uint32_t>::max());

    for (auto b : data) EXPECT_EQ(b, 0xFF);
}

TEST(NumViewTest, SetNum_Value250_RoundTrip) {
    std::vector<uint8_t> data(4, 0);
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    view.SetNum<uint32_t>(250);
    EXPECT_EQ(view.GetNum<uint32_t>(), 250u);
}

TEST(NumViewTest, SetNum_Uint8_RoundTrip) {
    std::vector<uint8_t> data(1, 0);
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    view.SetNum<uint8_t>(0xAB);
    EXPECT_EQ(view.GetNum<uint8_t>(), 0xAB);
}

TEST(NumViewTest, SetNum_Uint16_RoundTrip) {
    std::vector<uint8_t> data(2, 0);
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    view.SetNum<uint16_t>(12345);
    EXPECT_EQ(view.GetNum<uint16_t>(), 12345u);
}

TEST(NumViewTest, SetNum_Uint64_RoundTrip) {
    std::vector<uint8_t> data(8, 0);
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    uint64_t value = 0xDEADBEEFCAFEBABEULL;
    view.SetNum<uint64_t>(value);
    EXPECT_EQ(view.GetNum<uint64_t>(), value);
}

TEST(NumViewTest, SetNum_BE_RoundTrip) {
    std::vector<uint8_t> data(4, 0);
    wiseio::NumView view(data, wiseio::Endianness::kBigEndian);
    view.SetNum<uint32_t>(0xCAFEBABE);
    EXPECT_EQ(view.GetNum<uint32_t>(), 0xCAFEBABEu);
}

// ==================== SetNum изменяет вектор по ссылке ====================

TEST(NumViewTest, SetNum_ModifiesOriginalVector) {
    std::vector<uint8_t> data(4, 0);
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    view.SetNum<uint32_t>(42);

    EXPECT_NE(data, std::vector<uint8_t>({0, 0, 0, 0}));
}

// ==================== Негативные сценарии (через FromVector) ====================

TEST(NumViewTest, GetNum_SizeMismatch_Throws) {
    std::vector<uint8_t> data = {0x01, 0x02};
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);

    EXPECT_THROW(view.GetNum<uint32_t>(), std::logic_error);
}

TEST(NumViewTest, GetNum_EmptyVector_Throws) {
    std::vector<uint8_t> data;
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);

    EXPECT_THROW(view.GetNum<uint32_t>(), std::logic_error);
}

// ==================== DefaultEndianess ====================

TEST(NumViewTest, DefaultEndianess_IsLittleEndian) {
    std::vector<uint8_t> data = {0xFA, 0x00, 0x00, 0x00};
    wiseio::NumView view_default(data);
    wiseio::NumView view_le(data, wiseio::Endianness::kLittleEndian);

    EXPECT_EQ(view_default.GetNum<uint32_t>(), view_le.GetNum<uint32_t>());
    EXPECT_EQ(view_default.GetNum<uint32_t>(), 250u);
}

// NOLINTEND