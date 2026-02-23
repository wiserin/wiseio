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

// ==================== File-обёртка из задания ====================

class File {
    enum class Modules { kFirst = 0, kSecond, kThird };
    wiseio::ByteFile<Modules> file_;

public:
    explicit File(const char* name)
        : file_(std::move(wiseio::ByteFile<Modules>(name))) {
        file_.AddChunk(
            wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t),
            Modules::kFirst);
        file_.AddChunk(
            wiseio::MakeNumChunk(wiseio::NumSize::kUint32_t),
            Modules::kSecond);
        file_.AddChunk(
            wiseio::MakeByteChunk(wiseio::NumSize::kUint32_t),
            Modules::kThird);
    }

    wiseio::Storage& GetTemp() {
        return file_.GetChunk(Modules::kThird).GetStorage();
    }

    void InitFromFile() {
        file_.InitChunksFromFile();
    }

    void Compile() {
        file_.Compile();
    }

    wiseio::Storage& LoadFirstChunk() {
        return file_.GetAndLoadChunk(Modules::kFirst).GetStorage();
    }

    wiseio::Storage& LoadSecondChunk() {
        return file_.GetAndLoadChunk(Modules::kSecond).GetStorage();
    }

    wiseio::Storage& LoadThirdChunk() {
        return file_.GetAndLoadChunk(Modules::kThird).GetStorage();
    }
};

// ==================== Фикстура ====================

class FileWrapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = fs::temp_directory_path() / "wiseio_wrapper_tests";
        cache_dir_ = fs::temp_directory_path() / "wiseio_wrapper_cache";
        fs::create_directories(test_dir_);
        fs::create_directories(cache_dir_);
        wiseio::Storage::SetCacheDir(cache_dir_.string());
        logging::Logger::SetupLogger(logging::LoggerMode::kDebug, logging::LoggerIOMode::kSync, true);
    }

    void TearDown() override {
        if (fs::exists(test_dir_)) fs::remove_all(test_dir_);
        if (fs::exists(cache_dir_)) fs::remove_all(cache_dir_);
    }

    std::string CreateBinFile(
            const std::string& name,
            uint32_t first, uint32_t second,
            const std::vector<uint8_t>& payload = {}) {
        auto path = test_dir_ / name;
        std::ofstream f(path, std::ios::binary);

        auto write_u32 = [&](uint32_t v) {
            uint8_t b[4] = {
                static_cast<uint8_t>(v & 0xFF),
                static_cast<uint8_t>((v >> 8) & 0xFF),
                static_cast<uint8_t>((v >> 16) & 0xFF),
                static_cast<uint8_t>((v >> 24) & 0xFF)
            };
            f.write(reinterpret_cast<char*>(b), 4);
        };

        write_u32(first);
        write_u32(second);
        write_u32(static_cast<uint32_t>(payload.size()));
        f.write(reinterpret_cast<const char*>(payload.data()), payload.size());
        return path.string();
    }

    fs::path test_dir_;
    fs::path cache_dir_;
};

// ==================== Конструктор ====================

TEST_F(FileWrapperTest, Constructor_ValidFile_NoThrow) {
    auto path = CreateBinFile("ctor.bin", 1, 2);
    EXPECT_NO_THROW(File f(path.c_str()));
}

TEST_F(FileWrapperTest, Constructor_NonExistentFile_Throws) {
    EXPECT_THROW(File f("/no/such/path.bin"), std::runtime_error);
}

// ==================== InitFromFile ====================

TEST_F(FileWrapperTest, InitFromFile_NoThrow) {
    auto path = CreateBinFile("init.bin", 10, 20, {0x01});
    File f(path.c_str());
    EXPECT_NO_THROW(f.InitFromFile());
}

// ==================== LoadFirstChunk ====================

TEST_F(FileWrapperTest, LoadFirstChunk_Value42) {
    auto path = CreateBinFile("first42.bin", 42, 0);
    File f(path.c_str());
    f.InitFromFile();

    wiseio::Storage& st = f.LoadFirstChunk();
    wiseio::NumView view(st.GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 42u);
}

TEST_F(FileWrapperTest, LoadFirstChunk_Value250_ExactlyAsInExample) {
    auto path = CreateBinFile("example.bin", 250, 0);
    File f(path.c_str());
    f.InitFromFile();

    wiseio::Storage& st = f.LoadFirstChunk();
    std::vector<uint8_t>& buff = st.GetData();
    wiseio::NumView view(buff, wiseio::Endianness::kLittleEndian);

    uint32_t val = view.GetNum<uint32_t>();
    EXPECT_EQ(val, 250u);
}

TEST_F(FileWrapperTest, LoadFirstChunk_SetNum250_CommitAndVerify) {
    auto path = CreateBinFile("set250.bin", 100, 200, {0xAA});
    File f(path.c_str());
    f.InitFromFile();

    wiseio::Storage& st = f.LoadFirstChunk();
    wiseio::NumView view(st.GetData(), wiseio::Endianness::kLittleEndian);
    view.SetNum<uint32_t>(250);
    st.Commit();

    wiseio::NumView view2(st.GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view2.GetNum<uint32_t>(), 250u);
}

// ==================== LoadSecondChunk ====================

TEST_F(FileWrapperTest, LoadSecondChunk_CorrectValue) {
    auto path = CreateBinFile("second.bin", 0, 999);
    File f(path.c_str());
    f.InitFromFile();

    wiseio::NumView view(f.LoadSecondChunk().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(view.GetNum<uint32_t>(), 999u);
}

// ==================== GetTemp / LoadThirdChunk ====================

TEST_F(FileWrapperTest, LoadThirdChunk_CorrectPayload) {
    std::vector<uint8_t> payload = {0xDE, 0xAD, 0xBE, 0xEF};
    auto path = CreateBinFile("third.bin", 0, 0, payload);
    File f(path.c_str());
    f.InitFromFile();

    EXPECT_EQ(f.LoadThirdChunk().GetData(), payload);
}

TEST_F(FileWrapperTest, GetTemp_AfterInitFromFile_HasPayload) {
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
    auto path = CreateBinFile("temp.bin", 0, 0, payload);
    File f(path.c_str());
    f.InitFromFile();
    f.LoadThirdChunk();

    EXPECT_EQ(f.GetTemp().GetData(), payload);
}

TEST_F(FileWrapperTest, LoadThirdChunk_EmptyPayload) {
    auto path = CreateBinFile("temp_empty.bin", 1, 2);
    File f(path.c_str());
    f.InitFromFile();

    EXPECT_TRUE(f.LoadThirdChunk().GetData().empty());
}

// ==================== Compile ====================

TEST_F(FileWrapperTest, Compile_NoThrow) {
    auto path = CreateBinFile("compile.bin", 10, 20, {0x01});
    File f(path.c_str());
    f.InitFromFile();
    EXPECT_NO_THROW(f.Compile());
}

TEST_F(FileWrapperTest, Compile_PersistsModifiedFirstChunk) {
    auto path = CreateBinFile("persist_first.bin", 10, 20, {0x01});
    {
        File f(path.c_str());
        f.InitFromFile();

        wiseio::Storage& st = f.LoadFirstChunk();
        wiseio::NumView view(st.GetData(), wiseio::Endianness::kLittleEndian);
        view.SetNum<uint32_t>(777);
        st.Commit();

        f.Compile();
    }

    File f2(path.c_str());
    f2.InitFromFile();
    wiseio::NumView v(f2.LoadFirstChunk().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(v.GetNum<uint32_t>(), 777u);
}

TEST_F(FileWrapperTest, Compile_PersistsModifiedSecondChunk) {
    auto path = CreateBinFile("persist_second.bin", 1, 2);
    {
        File f(path.c_str());
        f.InitFromFile();

        wiseio::Storage& st = f.LoadSecondChunk();
        wiseio::NumView view(st.GetData(), wiseio::Endianness::kLittleEndian);
        view.SetNum<uint32_t>(555);
        st.Commit();

        f.Compile();
    }

    File f2(path.c_str());
    f2.InitFromFile();
    wiseio::NumView v(f2.LoadSecondChunk().GetData(), wiseio::Endianness::kLittleEndian);
    EXPECT_EQ(v.GetNum<uint32_t>(), 555u);
}

TEST_F(FileWrapperTest, Compile_AllChunksModified_AllPersist) {
    auto path = CreateBinFile("all_mods.bin", 1, 2, {0x00, 0x00, 0x00});
    {
        File f(path.c_str());
        f.InitFromFile();

        {
            wiseio::Storage& st = f.LoadFirstChunk();
            wiseio::NumView v(st.GetData(), wiseio::Endianness::kLittleEndian);
            v.SetNum<uint32_t>(111);
            st.Commit();
        }
        {
            wiseio::Storage& st = f.LoadSecondChunk();
            wiseio::NumView v(st.GetData(), wiseio::Endianness::kLittleEndian);
            v.SetNum<uint32_t>(222);
            st.Commit();
        }
        {
            wiseio::Storage& st = f.LoadThirdChunk();
            st.GetData() = {0xFF, 0xFE, 0xFD};
            st.Commit();
        }

        f.Compile();
    }

    File f2(path.c_str());
    f2.InitFromFile();

    wiseio::NumView v1(f2.LoadFirstChunk().GetData(), wiseio::Endianness::kLittleEndian);
    wiseio::NumView v2(f2.LoadSecondChunk().GetData(), wiseio::Endianness::kLittleEndian);

    EXPECT_EQ(v1.GetNum<uint32_t>(), 111u);
    EXPECT_EQ(v2.GetNum<uint32_t>(), 222u);
    EXPECT_EQ(f2.LoadThirdChunk().GetData(), std::vector<uint8_t>({0xFF, 0xFE, 0xFD}));
}

// ==================== Граничные значения ====================

TEST_F(FileWrapperTest, BoundaryValues_ZerosInNumChunks) {
    auto path = CreateBinFile("zeros.bin", 0, 0);
    File f(path.c_str());
    f.InitFromFile();

    wiseio::NumView v1(f.LoadFirstChunk().GetData(), wiseio::Endianness::kLittleEndian);
    wiseio::NumView v2(f.LoadSecondChunk().GetData(), wiseio::Endianness::kLittleEndian);

    EXPECT_EQ(v1.GetNum<uint32_t>(), 0u);
    EXPECT_EQ(v2.GetNum<uint32_t>(), 0u);
}

TEST_F(FileWrapperTest, BoundaryValues_MaxUint32InNumChunks) {
    auto path = CreateBinFile("maxvals.bin", 0xFFFFFFFF, 0xFFFFFFFF);
    File f(path.c_str());
    f.InitFromFile();

    wiseio::NumView v1(f.LoadFirstChunk().GetData(), wiseio::Endianness::kLittleEndian);
    wiseio::NumView v2(f.LoadSecondChunk().GetData(), wiseio::Endianness::kLittleEndian);

    EXPECT_EQ(v1.GetNum<uint32_t>(), 0xFFFFFFFFu);
    EXPECT_EQ(v2.GetNum<uint32_t>(), 0xFFFFFFFFu);
}

TEST_F(FileWrapperTest, BoundaryValues_LargePayload) {
    std::vector<uint8_t> payload(10000);
    for (size_t i = 0; i < payload.size(); ++i) payload[i] = static_cast<uint8_t>(i % 256);
    auto path = CreateBinFile("large_payload.bin", 1, 2, payload);

    File f(path.c_str());
    f.InitFromFile();

    EXPECT_EQ(f.LoadThirdChunk().GetData(), payload);
}

// ==================== Стресс: цикл чтения/изменения ====================

TEST_F(FileWrapperTest, StressTest_ManyReadWriteCycles) {
    auto path = CreateBinFile("stress.bin", 0, 0);

    for (uint32_t i = 0; i < 20; ++i) {
        {
            File f(path.c_str());
            f.InitFromFile();

            wiseio::Storage& st = f.LoadFirstChunk();
            wiseio::NumView view(st.GetData(), wiseio::Endianness::kLittleEndian);
            view.SetNum<uint32_t>(i);
            st.Commit();

            f.Compile();
        }

        File fcheck(path.c_str());
        fcheck.InitFromFile();
        wiseio::NumView v(fcheck.LoadFirstChunk().GetData(), wiseio::Endianness::kLittleEndian);
        EXPECT_EQ(v.GetNum<uint32_t>(), i);
    }
}

// NOLINTEND