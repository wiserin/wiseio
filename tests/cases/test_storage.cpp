// NOLINTBEGIN
#include <gtest/gtest.h>
#include <cstdint>
#include <filesystem>
#include <vector>

#include <logging/logger.hpp>
#include <logging/schemas.hpp>

#include "wise-io/byte/storage.hpp"

namespace fs = std::filesystem;

class StorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        cache_dir_ = fs::temp_directory_path() / "wiseio_storage_cache";
        fs::create_directories(cache_dir_);
        wiseio::Storage::SetCacheDir(cache_dir_.string());
        logging::Logger::SetupLogger(logging::LoggerMode::kDebug, logging::LoggerIOMode::kSync, true);
    }

    void TearDown() override {
        if (fs::exists(cache_dir_)) {
            fs::remove_all(cache_dir_);
        }
    }

    fs::path cache_dir_;
};

// ==================== GetData ====================

TEST_F(StorageTest, GetData_InitiallyEmpty) {
    wiseio::Storage storage;
    auto& data = storage.GetData();
    EXPECT_TRUE(data.empty());
}

TEST_F(StorageTest, GetData_ReturnsModifiableReference) {
    wiseio::Storage storage;
    auto& data = storage.GetData();
    data.push_back(0xAA);
    EXPECT_EQ(storage.GetData().size(), 1);
    EXPECT_EQ(storage.GetData()[0], 0xAA);
}

TEST_F(StorageTest, GetData_CanWriteAndRead) {
    wiseio::Storage storage;
    storage.GetData() = {0x01, 0x02, 0x03};
    auto& d = storage.GetData();
    EXPECT_EQ(d, std::vector<uint8_t>({0x01, 0x02, 0x03}));
}

// ==================== IsChanged ====================

TEST_F(StorageTest, IsChanged_InitiallyFalse) {
    wiseio::Storage storage;
    EXPECT_FALSE(storage.IsChanged());
}

TEST_F(StorageTest, IsChanged_TrueAfterGetData) {
    wiseio::Storage storage;
    (void)storage.GetData();
    EXPECT_TRUE(storage.IsChanged());
}

TEST_F(StorageTest, IsChanged_TrueAfterCommit) {
    wiseio::Storage storage;
    storage.GetData() = {0x01};
    storage.Commit();
    EXPECT_TRUE(storage.IsChanged());
}

// ==================== Commit ====================

TEST_F(StorageTest, Commit_NoThrow) {
    wiseio::Storage storage;
    storage.GetData() = {0xAA, 0xBB, 0xCC};
    EXPECT_NO_THROW(storage.Commit());
}

TEST_F(StorageTest, Commit_DataRoundTrip) {
    wiseio::Storage storage;
    storage.GetData() = {0x01, 0x02, 0x03};
    storage.Commit();

    auto& restored = storage.GetData();
    EXPECT_EQ(restored, std::vector<uint8_t>({0x01, 0x02, 0x03}));
}

TEST_F(StorageTest, Commit_EmptyData_NoThrow) {
    wiseio::Storage storage;
    (void)storage.GetData();
    EXPECT_NO_THROW(storage.Commit());
    EXPECT_TRUE(storage.GetData().empty());
}

TEST_F(StorageTest, Commit_LargeData_RoundTrip) {
    wiseio::Storage storage;
    auto& data = storage.GetData();
    data.resize(100000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i % 256);
    }
    storage.Commit();

    auto& restored = storage.GetData();
    EXPECT_EQ(restored.size(), 100000);
    for (size_t i = 0; i < restored.size(); ++i) {
        EXPECT_EQ(restored[i], static_cast<uint8_t>(i % 256));
    }
}

TEST_F(StorageTest, Commit_MultipleTimes_LastValueWins) {
    wiseio::Storage storage;

    for (uint8_t val = 1; val <= 5; ++val) {
        storage.GetData() = {val, static_cast<uint8_t>(val + 10)};
        storage.Commit();
    }

    EXPECT_EQ(storage.GetData(), std::vector<uint8_t>({5, 15}));
}

TEST_F(StorageTest, Commit_BinaryData_RoundTrip) {
    wiseio::Storage storage;
    std::vector<uint8_t> expected = {0x00, 0xFF, 0x01, 0xFE, 0x7F, 0x80};
    storage.GetData() = expected;
    storage.Commit();
    EXPECT_EQ(storage.GetData(), expected);
}

// ==================== SetCacheDir ====================

TEST_F(StorageTest, SetCacheDir_ValidDir_NoThrow) {
    EXPECT_NO_THROW(wiseio::Storage::SetCacheDir(cache_dir_.string()));
}

TEST_F(StorageTest, SetCacheDir_InvalidDir_Throws) {
    EXPECT_THROW(
        wiseio::Storage::SetCacheDir("/nonexistent/path/that/does/not/exist"),
        std::runtime_error
    );
}

// ==================== Move semantics ====================

TEST_F(StorageTest, MoveConstructor_WorksCorrectly) {
    wiseio::Storage storage1;
    storage1.GetData() = {0x10, 0x20, 0x30};

    wiseio::Storage storage2(std::move(storage1));
    EXPECT_EQ(storage2.GetData(), std::vector<uint8_t>({0x10, 0x20, 0x30}));
}

TEST_F(StorageTest, MoveAssignment_WorksCorrectly) {
    wiseio::Storage storage1;
    storage1.GetData() = {0xAA, 0xBB};

    wiseio::Storage storage2;
    storage2 = std::move(storage1);

    EXPECT_EQ(storage2.GetData(), std::vector<uint8_t>({0xAA, 0xBB}));
}

// NOLINTEND