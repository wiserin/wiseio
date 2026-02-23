// NOLINTBEGIN
#include <gtest/gtest.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

#include <logging/logger.hpp>
#include <logging/schemas.hpp>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/byte/storage.hpp"
#include "wise-io/byte/views.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"

namespace fs = std::filesystem;

// ==================== Утилиты ====================

static void WriteU32LE(std::ofstream& f, uint32_t v) {
    uint8_t b[4] = {
        static_cast<uint8_t>(v & 0xFF),
        static_cast<uint8_t>((v >> 8) & 0xFF),
        static_cast<uint8_t>((v >> 16) & 0xFF),
        static_cast<uint8_t>((v >> 24) & 0xFF)
    };
    f.write(reinterpret_cast<char*>(b), 4);
}

static void WriteBytes(std::ofstream& f, const std::vector<uint8_t>& data) {
    f.write(reinterpret_cast<const char*>(data.data()), data.size());
}

class ChunkTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "wiseio_chunk_tests";
        cache_dir_ = fs::temp_directory_path() / "wiseio_chunk_cache";
        fs::create_directories(test_dir_);
        fs::create_directories(cache_dir_);
        wiseio::Storage::SetCacheDir(cache_dir_.string());
        logging::Logger::SetupLogger(logging::LoggerMode::kDebug, logging::LoggerIOMode::kSync, true);
    }

    void TearDown() override {
        if (fs::exists(test_dir_)) fs::remove_all(test_dir_);
        if (fs::exists(cache_dir_)) fs::remove_all(cache_dir_);
    }

    std::string MakeNumChunkFile(const std::string& name, uint32_t value) {
        auto path = test_dir_ / name;
        std::ofstream f(path, std::ios::binary);
        WriteU32LE(f, value);
        return path.string();
    }

    std::string MakeByteChunkFile(const std::string& name, const std::vector<uint8_t>& payload) {
        auto path = test_dir_ / name;
        std::ofstream f(path, std::ios::binary);
        WriteU32LE(f, static_cast<uint32_t>(payload.size()));
        WriteBytes(f, payload);
        return path.string();
    }

    std::string MakeMultiChunkFile(
            const std::string& name,
            uint32_t first, uint32_t second,
            const std::vector<uint8_t>& payload) {
        auto path = test_dir_ / name;
        std::ofstream f(path, std::ios::binary);
        WriteU32LE(f, first);
        WriteU32LE(f, second);
        WriteU32LE(f, static_cast<uint32_t>(payload.size()));
        WriteBytes(f, payload);
        return path.string();
    }

    std::string MakeValidateChunkFile(
            const std::string& name,
            const std::vector<uint8_t>& magic) {
        auto path = test_dir_ / name;
        std::ofstream f(path, std::ios::binary);
        WriteBytes(f, magic);
        return path.string();
    }

    fs::path test_dir_;
    fs::path cache_dir_;
};

// ==================== MakeNumChunk ====================

TEST_F(ChunkTest, MakeNumChunk_Uint8_NotNull) {
    EXPECT_NE(wiseio::MakeNumChunk(wiseio::NumSize::kUint8_t), nullptr);
}

TEST_F(ChunkTest, MakeNumChunk_Uint16_NotNull) {
    EXPECT_NE(wiseio::MakeNumChunk(wiseio::NumSize::kUint16_t), nullptr);
}

TEST_F(ChunkTest, MakeNumChunk_Uint32_NotNull) {
    EXPECT_NE(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), nullptr);
}

TEST_F(ChunkTest, MakeNumChunk_Uint64_NotNull) {
    EXPECT_NE(wiseio::MakeNumChunk(wiseio::NumSize::kUint64_t), nullptr);
}

// ==================== MakeByteChunk ====================

TEST_F(ChunkTest, MakeByteChunk_Uint32_LE_NotNull) {
    EXPECT_NE(
        wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t, wiseio::Endianness::kLittleEndian),
        nullptr);
}

TEST_F(ChunkTest, MakeByteChunk_Uint32_BE_NotNull) {
    EXPECT_NE(
        wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t, wiseio::Endianness::kBigEndian),
        nullptr);
}

TEST_F(ChunkTest, MakeByteChunk_DefaultEndianess_NotNull) {
    EXPECT_NE(wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t), nullptr);
}

// ==================== MakeValidateChunk ====================

TEST_F(ChunkTest, MakeValidateChunk_NotNull) {
    EXPECT_NE(
        wiseio::MakeValidateChunk(4, {0xDE, 0xAD, 0xBE, 0xEF}),
        nullptr);
}

// ==================== IsInitialized ====================

TEST_F(ChunkTest, NumChunk_NotInitialized_Initially) {
    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    EXPECT_FALSE(chunk->IsInitialized());
}

TEST_F(ChunkTest, ByteChunk_NotInitialized_Initially) {
    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    EXPECT_FALSE(chunk->IsInitialized());
}

// ==================== NumChunk Init + Load ====================

TEST_F(ChunkTest, NumChunk_Init_SetsInitialized) {
    auto path = MakeNumChunkFile("num_init.bin", 42);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);

    EXPECT_TRUE(chunk->IsInitialized());
}

TEST_F(ChunkTest, NumChunk_Load_WithoutInit_Throws) {
    auto path = MakeNumChunkFile("num_no_init.bin", 42);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    EXPECT_THROW(chunk->Load(stream), std::runtime_error);
}

TEST_F(ChunkTest, NumChunk_InitAndLoad_Value42) {
    auto path = MakeNumChunkFile("num_42.bin", 42);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    auto& data = chunk->GetStorage().GetData();
    EXPECT_EQ(data.size(), 4u);
    wiseio::NumView view(data, wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 42u);
}

TEST_F(ChunkTest, NumChunk_InitAndLoad_Value250) {
    auto path = MakeNumChunkFile("num_250.bin", 250);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    wiseio::NumView view(chunk->GetStorage().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 250u);
}

TEST_F(ChunkTest, NumChunk_InitAndLoad_Zero) {
    auto path = MakeNumChunkFile("num_zero.bin", 0);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    wiseio::NumView view(chunk->GetStorage().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 0u);
}

TEST_F(ChunkTest, NumChunk_InitAndLoad_MaxUint32) {
    auto path = MakeNumChunkFile("num_max.bin", 0xFFFFFFFF);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    wiseio::NumView view(chunk->GetStorage().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 0xFFFFFFFFu);
}

TEST_F(ChunkTest, NumChunk_GetOffset_AfterInit) {
    auto path = MakeNumChunkFile("num_offset.bin", 1);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);

    EXPECT_EQ(chunk->GetOffset(), 0u);
}

TEST_F(ChunkTest, NumChunk_GetSize_Uint32) {
    auto path = MakeNumChunkFile("num_size.bin", 0);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);

    EXPECT_EQ(chunk->GetSize(), 4u);
}

// ==================== ByteChunk Init + Load ====================

TEST_F(ChunkTest, ByteChunk_Load_WithoutInit_Throws) {
    auto path = MakeByteChunkFile("byte_no_init.bin", {0x01});
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    EXPECT_THROW(chunk->Load(stream), std::runtime_error);
}

TEST_F(ChunkTest, ByteChunk_Init_SetsInitialized) {
    auto path = MakeByteChunkFile("byte_init.bin", {0x01, 0x02});
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);

    EXPECT_TRUE(chunk->IsInitialized());
}

TEST_F(ChunkTest, ByteChunk_InitAndLoad_SmallPayload) {
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03, 0x04, 0x05};
    auto path = MakeByteChunkFile("byte_small.bin", payload);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    EXPECT_EQ(chunk->GetStorage().GetData(), payload);
}

TEST_F(ChunkTest, ByteChunk_InitAndLoad_EmptyPayload) {
    auto path = MakeByteChunkFile("byte_empty.bin", {});
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    EXPECT_TRUE(chunk->GetStorage().GetData().empty());
}

TEST_F(ChunkTest, ByteChunk_InitAndLoad_BinaryData) {
    std::vector<uint8_t> payload = {0x00, 0xFF, 0x01, 0xFE, 0x7F, 0x80};
    auto path = MakeByteChunkFile("byte_binary.bin", payload);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    EXPECT_EQ(chunk->GetStorage().GetData(), payload);
}

TEST_F(ChunkTest, ByteChunk_InitAndLoad_LargePayload) {
    std::vector<uint8_t> payload(1000);
    for (size_t i = 0; i < payload.size(); ++i) payload[i] = static_cast<uint8_t>(i % 256);
    auto path = MakeByteChunkFile("byte_large.bin", payload);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    EXPECT_EQ(chunk->GetStorage().GetData(), payload);
}

TEST_F(ChunkTest, ByteChunk_GetSize_AfterInit) {
    std::vector<uint8_t> payload = {1, 2, 3};
    auto path = MakeByteChunkFile("byte_size.bin", payload);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);

    EXPECT_EQ(chunk->GetSize(), 3u);
}

// ==================== ValidateChunk Init ====================

TEST_F(ChunkTest, ValidateChunk_Init_MatchingBytes_NoThrow) {
    std::vector<uint8_t> magic = {0xDE, 0xAD, 0xBE, 0xEF};
    auto path = MakeValidateChunkFile("validate_ok.bin", magic);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeValidateChunk(4, {0xDE, 0xAD, 0xBE, 0xEF});
    EXPECT_NO_THROW(chunk->Init(stream));
    EXPECT_TRUE(chunk->IsInitialized());
}

TEST_F(ChunkTest, ValidateChunk_Init_MismatchBytes_Throws) {
    std::vector<uint8_t> actual = {0x01, 0x02, 0x03, 0x04};
    auto path = MakeValidateChunkFile("validate_fail.bin", actual);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeValidateChunk(4, {0xDE, 0xAD, 0xBE, 0xEF});
    EXPECT_THROW(chunk->Init(stream), std::logic_error);
}

TEST_F(ChunkTest, ValidateChunk_Load_WithoutInit_Throws) {
    std::vector<uint8_t> magic = {0xCA, 0xFE};
    auto path = MakeValidateChunkFile("validate_no_init.bin", magic);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeValidateChunk(2, {0xCA, 0xFE});
    EXPECT_THROW(chunk->Load(stream), std::runtime_error);
}

TEST_F(ChunkTest, ValidateChunk_InitAndLoad_ReadsData) {
    std::vector<uint8_t> magic = {0xCA, 0xFE, 0xBA, 0xBE};
    auto path = MakeValidateChunkFile("validate_load.bin", magic);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeValidateChunk(4, std::vector<uint8_t>(magic));
    chunk->Init(stream);
    chunk->Load(stream);

    EXPECT_EQ(chunk->GetStorage().GetData(), magic);
}

TEST_F(ChunkTest, ValidateChunk_GetSize) {
    auto chunk = wiseio::MakeValidateChunk(4, {0x01, 0x02, 0x03, 0x04});
    EXPECT_EQ(chunk->GetSize(), 4u);
}

// ==================== GetCompiledChunk ====================

TEST_F(ChunkTest, NumChunk_GetCompiledChunk_EqualsRawData) {
    auto path = MakeNumChunkFile("num_compile.bin", 0x12345678);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    auto compiled = chunk->GetCompiledChunk();
    EXPECT_EQ(compiled.size(), 4u);

    EXPECT_EQ(compiled[0], 0x78);
    EXPECT_EQ(compiled[1], 0x56);
    EXPECT_EQ(compiled[2], 0x34);
    EXPECT_EQ(compiled[3], 0x12);
}

TEST_F(ChunkTest, ByteChunk_GetCompiledChunk_HasLenPrefix) {
    std::vector<uint8_t> payload = {0xAA, 0xBB, 0xCC};
    auto path = MakeByteChunkFile("byte_compile.bin", payload);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);
    chunk->Init(stream);
    chunk->Load(stream);

    auto compiled = chunk->GetCompiledChunk();

    EXPECT_EQ(compiled.size(), 7u);

    EXPECT_EQ(compiled[0], 0x03);
    EXPECT_EQ(compiled[1], 0x00);
    EXPECT_EQ(compiled[2], 0x00);
    EXPECT_EQ(compiled[3], 0x00);

    EXPECT_EQ(compiled[4], 0xAA);
    EXPECT_EQ(compiled[5], 0xBB);
    EXPECT_EQ(compiled[6], 0xCC);
}

// ==================== Последовательная загрузка нескольких чанков ====================

TEST_F(ChunkTest, MultiChunk_SequentialInitAndLoad) {
    std::vector<uint8_t> payload = {0xCA, 0xFE, 0xBA, 0xBE};
    auto path = MakeMultiChunkFile("multi.bin", 100, 200, payload);
    auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kReadAndWrite);

    auto chunk1 = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    auto chunk2 = wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t);
    auto chunk3 = wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t);

    chunk1->Init(stream);
    chunk2->Init(stream);
    chunk3->Init(stream);

    chunk1->Load(stream);
    chunk2->Load(stream);
    chunk3->Load(stream);

    wiseio::NumView v1(chunk1->GetStorage().GetData(), wiseio::Endianness::kLittleEndian);
    wiseio::NumView v2(chunk2->GetStorage().GetData(), wiseio::Endianness::kLittleEndian);

    EXPECT_EQ(v1.GetNum<uint32_t>(), 100u);
    EXPECT_EQ(v2.GetNum<uint32_t>(), 200u);
    EXPECT_EQ(chunk3->GetStorage().GetData(), payload);
}

// NOLINTEND