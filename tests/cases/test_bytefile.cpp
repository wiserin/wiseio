// NOLINTBEGIN
#include <gtest/gtest.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <logging/logger.hpp>
#include <logging/schemas.hpp>

#include "wise-io/byte/bytefile.hpp"
#include "wise-io/byte/chunks.hpp"
#include "wise-io/byte/storage.hpp"
#include "wise-io/byte/views.hpp"
#include "wise-io/schemas.hpp"

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

// ==================== Фикстура ====================

class ByteFileTest : public ::testing::Test {
protected:
    enum class Slots { kFirst = 0, kSecond, kThird };

    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "wiseio_bytefile_tests";
        cache_dir_ = fs::temp_directory_path() / "wiseio_bytefile_cache";
        fs::create_directories(test_dir_);
        fs::create_directories(cache_dir_);
        wiseio::Storage::SetCacheDir(cache_dir_.string());
        logging::Logger::SetupLogger(logging::LoggerMode::kDebug, logging::LoggerIOMode::kSync, true);
    }

    void TearDown() override {
        if (fs::exists(test_dir_)) fs::remove_all(test_dir_);
        if (fs::exists(cache_dir_)) fs::remove_all(cache_dir_);
    }

    std::string CreateFile(
            const std::string& name,
            uint32_t first, uint32_t second,
            const std::vector<uint8_t>& payload = {}) {
        auto path = test_dir_ / name;
        std::ofstream f(path, std::ios::binary);
        WriteU32LE(f, first);
        WriteU32LE(f, second);
        WriteU32LE(f, static_cast<uint32_t>(payload.size()));
        f.write(reinterpret_cast<const char*>(payload.data()), payload.size());
        return path.string();
    }

    wiseio::ByteFile<Slots> MakeFile(const std::string& path) {
        wiseio::ByteFile<Slots> file(path.c_str());
        file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), Slots::kFirst);
        file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), Slots::kSecond);
        file.AddChunk(wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t), Slots::kThird);
        return file;
    }

    fs::path test_dir_;
    fs::path cache_dir_;
};

// ==================== Конструктор ====================

TEST_F(ByteFileTest, Constructor_ValidFile_NoThrow) {
    auto path = CreateFile("ctor.bin", 1, 2, {0xAA});
    EXPECT_NO_THROW(wiseio::ByteFile<Slots> f(path.c_str()));
}

TEST_F(ByteFileTest, Constructor_NonExistentFile_Throws) {
    EXPECT_THROW(
        wiseio::ByteFile<Slots> f("/nonexistent/path/file.bin"),
        std::runtime_error
    );
}

// ==================== AddChunk ====================

TEST_F(ByteFileTest, AddChunk_ThreeChunks_NoThrow) {
    auto path = CreateFile("add.bin", 1, 2, {0xBB});
    EXPECT_NO_THROW(MakeFile(path));
}

TEST_F(ByteFileTest, AddChunk_DuplicateKey_Throws) {
    auto path = CreateFile("dup.bin", 1, 2, {0xCC});
    wiseio::ByteFile<Slots> file(path.c_str());
    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), Slots::kFirst);

    EXPECT_THROW(
        file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), Slots::kFirst),
        std::logic_error
    );
}

TEST_F(ByteFileTest, AddChunk_ConstRefKey_Works) {
    auto path = CreateFile("const_ref.bin", 1, 2, {});
    wiseio::ByteFile<Slots> file(path.c_str());
    const Slots key = Slots::kFirst;
    EXPECT_NO_THROW(
        file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), key)
    );
}

// ==================== GetChunk ====================

TEST_F(ByteFileTest, GetChunk_ExistingKey_NoThrow) {
    auto path = CreateFile("get.bin", 10, 20, {0x01});
    auto file = MakeFile(path);

    EXPECT_NO_THROW({ auto& c = file.GetChunk(Slots::kFirst); (void)c; });
}

TEST_F(ByteFileTest, GetChunk_NonExistentKey_Throws) {
    auto path = CreateFile("nokey.bin", 1, 2, {0x01});
    wiseio::ByteFile<Slots> file(path.c_str());

    EXPECT_THROW(file.GetChunk(Slots::kFirst), std::logic_error);
}

TEST_F(ByteFileTest, GetChunk_ReturnsConsistentRef) {
    auto path = CreateFile("ref.bin", 1, 2, {0x01});
    auto file = MakeFile(path);

    auto& c1 = file.GetChunk(Slots::kFirst);
    auto& c2 = file.GetChunk(Slots::kFirst);
    EXPECT_EQ(&c1, &c2);
}

// ==================== InitChunksFromFile ====================

TEST_F(ByteFileTest, InitChunksFromFile_NoThrow) {
    auto path = CreateFile("init.bin", 42, 99, {0xDE, 0xAD});
    auto file = MakeFile(path);
    EXPECT_NO_THROW(file.InitChunksFromFile());
}

TEST_F(ByteFileTest, InitChunksFromFile_SetsInitialized) {
    auto path = CreateFile("init_state.bin", 1, 2, {});
    auto file = MakeFile(path);
    file.InitChunksFromFile();

    EXPECT_TRUE(file.GetChunk(Slots::kFirst).IsInitialized());
    EXPECT_TRUE(file.GetChunk(Slots::kSecond).IsInitialized());
    EXPECT_TRUE(file.GetChunk(Slots::kThird).IsInitialized());
}

// ==================== GetAndLoadChunk ====================

TEST_F(ByteFileTest, GetAndLoadChunk_AfterInit_NoThrow) {
    auto path = CreateFile("load_one.bin", 77, 88, {0xAA});
    auto file = MakeFile(path);
    file.InitChunksFromFile();

    EXPECT_NO_THROW({ auto& c = file.GetAndLoadChunk(Slots::kFirst); (void)c; });
}

TEST_F(ByteFileTest, GetAndLoadChunk_NonExistentKey_Throws) {
    auto path = CreateFile("load_nokey.bin", 1, 2, {0x01});
    wiseio::ByteFile<Slots> file(path.c_str()); // без AddChunk

    EXPECT_THROW(file.GetAndLoadChunk(Slots::kFirst), std::logic_error);
}

TEST_F(ByteFileTest, GetAndLoadChunk_FirstChunk_CorrectValue) {
    auto path = CreateFile("load_first.bin", 250, 0, {});
    auto file = MakeFile(path);
    file.InitChunksFromFile();

    auto& chunk = file.GetAndLoadChunk(Slots::kFirst);
    wiseio::NumView view(chunk.GetStorage().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 250u);
}

TEST_F(ByteFileTest, GetAndLoadChunk_SecondChunk_CorrectValue) {
    auto path = CreateFile("load_second.bin", 0, 999, {});
    auto file = MakeFile(path);
    file.InitChunksFromFile();

    auto& chunk = file.GetAndLoadChunk(Slots::kSecond);
    wiseio::NumView view(chunk.GetStorage().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 999u);
}

TEST_F(ByteFileTest, GetAndLoadChunk_ThirdChunk_CorrectPayload) {
    std::vector<uint8_t> payload = {0xCA, 0xFE, 0xBA, 0xBE};
    auto path = CreateFile("load_third.bin", 0, 0, payload);
    auto file = MakeFile(path);
    file.InitChunksFromFile();

    auto& chunk = file.GetAndLoadChunk(Slots::kThird);
    EXPECT_EQ(chunk.GetStorage().GetData(), payload);
}

// ==================== Compile ====================

TEST_F(ByteFileTest, Compile_NoThrow) {
    auto path = CreateFile("compile.bin", 10, 20, {0x01, 0x02});
    auto file = MakeFile(path);
    file.InitChunksFromFile();
    EXPECT_NO_THROW(file.Compile());
}

TEST_F(ByteFileTest, Compile_ThenReread_PreservesUnchangedValues) {
    auto path = CreateFile("compile_preserve.bin", 55, 66, {0xAA, 0xBB});
    {
        auto file = MakeFile(path);
        file.InitChunksFromFile();
        file.Compile();
    }

    auto file2 = MakeFile(path);
    file2.InitChunksFromFile();

    wiseio::NumView v1(file2.GetAndLoadChunk(Slots::kFirst).GetStorage().GetData(),
                       wiseio::Endianness::kLittleEndian);
    wiseio::NumView v2(file2.GetAndLoadChunk(Slots::kSecond).GetStorage().GetData(),
                       wiseio::Endianness::kLittleEndian);

    EXPECT_EQ(v1.GetNum<uint32_t>(), 55u);
    EXPECT_EQ(v2.GetNum<uint32_t>(), 66u);
    EXPECT_EQ(file2.GetAndLoadChunk(Slots::kThird).GetStorage().GetData(),
              std::vector<uint8_t>({0xAA, 0xBB}));
}

TEST_F(ByteFileTest, Compile_AfterModificationAndCommit_PersistsChange) {
    auto path = CreateFile("compile_modify.bin", 10, 20, {0x01});
    {
        auto file = MakeFile(path);
        file.InitChunksFromFile();

        auto& storage = file.GetAndLoadChunk(Slots::kFirst).GetStorage();
        wiseio::NumView view(storage.GetData(), wiseio::Endianness::kLittleEndian);
        view.SetNum<uint32_t>(999);
        storage.Commit();

        file.Compile();
    }

    auto file2 = MakeFile(path);
    file2.InitChunksFromFile();
    wiseio::NumView v(file2.GetAndLoadChunk(Slots::kFirst).GetStorage().GetData(),
                      wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(v.GetNum<uint32_t>(), 999u);
}

// ==================== String-ключ (ByteFile<std::string>) ====================

TEST_F(ByteFileTest, StringKey_AddAndGet_Works) {
    auto path = CreateFile("str_key.bin", 1, 2, {0x01});
    wiseio::ByteFile<std::string> file(path.c_str());
    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), std::string("first"));
    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), std::string("second"));
    file.AddChunk(wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t), std::string("third"));

    EXPECT_NO_THROW({ auto& c = file.GetChunk("first"); (void)c; });
}

TEST_F(ByteFileTest, StringKey_DuplicateKey_Throws) {
    auto path = CreateFile("str_dup.bin", 1, 2, {});
    wiseio::ByteFile<std::string> file(path.c_str());
    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), std::string("key"));

    EXPECT_THROW(
        file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), std::string("key")),
        std::logic_error
    );
}

TEST_F(ByteFileTest, StringKey_InitAndLoad_CorrectValues) {
    auto path = CreateFile("str_load.bin", 42, 0, {});
    wiseio::ByteFile<std::string> file(path.c_str());
    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), std::string("first"));
    file.AddChunk(wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t), std::string("second"));
    file.AddChunk(wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t), std::string("third"));

    file.InitChunksFromFile();
    auto& chunk = file.GetAndLoadChunk(std::string("first"));
    wiseio::NumView view(chunk.GetStorage().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 42u);
}

// ==================== Move semantics ====================

TEST_F(ByteFileTest, MoveConstructor_Works) {
    auto path = CreateFile("move.bin", 1, 2, {});
    auto file1 = MakeFile(path);
    wiseio::ByteFile<Slots> file2(std::move(file1));

    EXPECT_NO_THROW(file2.InitChunksFromFile());
}

TEST_F(ByteFileTest, MoveAssignment_Works) {
    auto path = CreateFile("move_assign.bin", 1, 2, {});
    auto file1 = MakeFile(path);
    wiseio::ByteFile<Slots> file2;
    file2 = std::move(file1);

    EXPECT_NO_THROW(file2.InitChunksFromFile());
}

// NOLINTEND