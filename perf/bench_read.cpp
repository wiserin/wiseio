// WiseIO vs fstream — сравнение скорости чтения структурированных бинарных файлов
// NOLINTBEGIN
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <logging/logger.hpp>
#include <logging/schemas.hpp>

#include "wise-io/byte/storage.hpp"
#include "wise-io/byte/views.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;
using Ms = std::chrono::duration<double, std::milli>;

// ==================== ANSI цвета (отключаются если NO_COLOR) ====================

static bool use_color() {
    return std::getenv("NO_COLOR") == nullptr && std::getenv("CI") == nullptr;
}

static const char* GREEN  = "";
static const char* YELLOW = "";
static const char* CYAN   = "";
static const char* RED    = "";
static const char* BOLD   = "";
static const char* RESET  = "";

static void InitColors() {
    if (use_color()) {
        GREEN  = "\033[32m";
        YELLOW = "\033[33m";
        CYAN   = "\033[36m";
        RED    = "\033[31m";
        BOLD   = "\033[1m";
        RESET  = "\033[0m";
    }
}

// ==================== Утилиты ====================

static void WriteU32LE(std::ofstream& f, uint32_t v) {
    uint8_t b[4] = {
        static_cast<uint8_t>(v),
        static_cast<uint8_t>(v >> 8),
        static_cast<uint8_t>(v >> 16),
        static_cast<uint8_t>(v >> 24)
    };
    f.write(reinterpret_cast<char*>(b), 4);
}

static double Measure(int warmup, int iters, std::function<void()> func) {
    for (int i = 0; i < warmup; ++i) func();

    std::vector<double> times;
    times.reserve(iters);
    for (int i = 0; i < iters; ++i) {
        auto t0 = Clock::now();
        func();
        auto t1 = Clock::now();
        times.push_back(Ms(t1 - t0).count());
    }
    std::sort(times.begin(), times.end());
    return times[times.size() / 2]; // медиана
}

static double MBps(size_t bytes, double ms) {
    return (static_cast<double>(bytes) / (1024.0 * 1024.0)) / (ms / 1000.0);
}

// ==================== Генерация тестовых файлов ====================

static std::string MakeRawFile(const fs::path& dir, const std::string& name, size_t size_bytes) {
    auto path = dir / name;
    std::ofstream f(path, std::ios::binary);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<uint8_t> buf(4096);
    size_t written = 0;
    while (written < size_bytes) {
        size_t chunk = std::min(buf.size(), size_bytes - written);
        for (size_t i = 0; i < chunk; ++i) buf[i] = static_cast<uint8_t>(dist(rng));
        f.write(reinterpret_cast<char*>(buf.data()), chunk);
        written += chunk;
    }
    return path.string();
}

static std::string MakeStructuredFile(
        const fs::path& dir,
        const std::string& name,
        int num_chunks,
        size_t payload_size) {
    auto path = dir / name;
    std::ofstream f(path, std::ios::binary);
    std::vector<uint8_t> payload(payload_size, 0xAB);
    for (int i = 0; i < num_chunks; ++i) {
        WriteU32LE(f, static_cast<uint32_t>(i));
        WriteU32LE(f, static_cast<uint32_t>(payload_size));
        f.write(reinterpret_cast<char*>(payload.data()), payload_size);
    }
    return path.string();
}

// ==================== Результат одного бенча ====================

struct BenchResult {
    std::string name;
    std::string impl;
    double ms;
    double mbps;
};

// ==================== Вывод результатов ====================

static std::string SpeedBar(double ratio, int width = 20) {
    int filled = static_cast<int>(std::min(ratio, 2.0) / 2.0 * width);
    std::string bar = "[";
    for (int i = 0; i < width; ++i) bar += (i < filled ? "#" : "-");
    bar += "]";
    return bar;
}

static void PrintHeader() {
    std::cout << "\n";
    std::cout << BOLD << CYAN;
    std::cout << "╔══════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              WiseIO  vs  fstream  —  Performance Benchmark              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
}

static void PrintSectionHeader(const std::string& title) {
    std::cout << BOLD << YELLOW << "  ┌─ " << title << " " << RESET << "\n";
}

static void PrintTableHeader() {
    std::cout << BOLD;
    std::cout << "  │  " << std::left << std::setw(14) << "Impl"
              << std::setw(12) << "Median ms"
              << std::setw(12) << "MB/s"
              << "Speedup vs fstream\n";
    std::cout << "  │  " << std::string(60, '-') << "\n";
    std::cout << RESET;
}

static void PrintRow(const BenchResult& r, double fstream_mbps) {
    bool is_wiseio = (r.impl == "WiseIO");
    double speedup = r.mbps / fstream_mbps;

    std::cout << "  │  ";
    std::cout << (is_wiseio ? CYAN : "") << BOLD
              << std::left << std::setw(14) << r.impl
              << RESET;
    std::cout << std::setw(12) << std::fixed << std::setprecision(3) << r.ms;

    if (is_wiseio) {
        std::cout << (speedup >= 1.0 ? GREEN : RED);
    }
    std::cout << std::setw(12) << std::fixed << std::setprecision(1) << r.mbps;
    if (is_wiseio) std::cout << RESET;

    if (is_wiseio) {
        std::string bar = SpeedBar(speedup);
        std::string speedup_str = std::to_string(static_cast<int>(speedup * 100)) + "%";
        std::cout << (speedup >= 1.0 ? GREEN : RED)
                  << bar << " " << speedup_str
                  << RESET;
    } else {
        std::cout << "baseline";
    }
    std::cout << "\n";
}

static void PrintSectionFooter() {
    std::cout << "  └\n\n";
}

static void PrintSummary(
        const std::string& scenario,
        double wiseio_mbps,
        double fstream_mbps) {
    double ratio = wiseio_mbps / fstream_mbps;
    bool faster = ratio >= 1.0;
    std::cout << "  "
              << (faster ? GREEN : RED) << BOLD
              << (faster ? "  ✓" : "  ✗") << " [" << scenario << "] "
              << "WiseIO is " << std::fixed << std::setprecision(1)
              << (faster ? ratio : 1.0 / ratio) << "x "
              << (faster ? "faster" : "slower")
              << " than fstream"
              << RESET << "\n";
}

// ==================== Бенчмарки ====================

static std::pair<BenchResult, BenchResult> BenchSequential(
        const std::string& path,
        size_t file_size,
        int warmup, int iters) {

    double fstream_ms = Measure(warmup, iters, [&]() {
        std::ifstream f(path, std::ios::binary);
        std::vector<char> buf(file_size);
        f.read(buf.data(), file_size);
    });

    double wiseio_ms = Measure(warmup, iters, [&]() {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        std::vector<uint8_t> buf(file_size);
        stream.ReadAll(buf);
    });

    return {
        {"sequential", "fstream", fstream_ms, MBps(file_size, fstream_ms)},
        {"sequential", "WiseIO",  wiseio_ms,  MBps(file_size, wiseio_ms)}
    };
}

enum class ChunkKey { kNum = 0, kByte };

static std::pair<BenchResult, BenchResult> BenchChunked(
        const std::string& path,
        int num_chunks,
        size_t payload_size,
        int warmup, int iters) {

    size_t total = static_cast<size_t>(num_chunks) * (4 + 4 + payload_size);

    double fstream_ms = Measure(warmup, iters, [&]() {
        std::ifstream f(path, std::ios::binary);
        for (int i = 0; i < num_chunks; ++i) {
            uint32_t num = 0;
            uint32_t len = 0;
            f.read(reinterpret_cast<char*>(&num), 4);
            f.read(reinterpret_cast<char*>(&len), 4);
            std::vector<uint8_t> payload(len);
            f.read(reinterpret_cast<char*>(payload.data()), len);
        }
    });

    double wiseio_ms = Measure(warmup, iters, [&]() {
        std::ifstream f(path, std::ios::binary);
        for (int i = 0; i < num_chunks; ++i) {
            size_t offset = static_cast<size_t>(i) * (4 + 4 + payload_size);
            auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);

            std::vector<uint8_t> num_buf(4);
            stream.CustomRead(num_buf, offset);

            std::vector<uint8_t> len_buf(4);
            stream.CustomRead(len_buf, offset + 4);

            wiseio::NumView len_view(len_buf, wiseio::Endianness::kLittleEndian);
            uint32_t len = len_view.GetNum<uint32_t>();

            std::vector<uint8_t> payload(len);
            stream.CustomRead(payload, offset + 8);
        }
    });

    return {
        {"chunked", "fstream", fstream_ms, MBps(total, fstream_ms)},
        {"chunked", "WiseIO",  wiseio_ms,  MBps(total, wiseio_ms)}
    };
}

static std::pair<BenchResult, BenchResult> BenchRandomAccess(
        const std::string& path,
        size_t file_size,
        int n_reads,
        size_t read_size,
        int warmup, int iters) {

    std::mt19937 rng(1337);
    std::uniform_int_distribution<size_t> dist(0, file_size - read_size - 1);
    std::vector<size_t> offsets(n_reads);
    for (auto& o : offsets) o = dist(rng);

    size_t total = static_cast<size_t>(n_reads) * read_size;

    double fstream_ms = Measure(warmup, iters, [&]() {
        std::ifstream f(path, std::ios::binary);
        std::vector<char> buf(read_size);
        for (size_t off : offsets) {
            f.seekg(static_cast<std::streamoff>(off));
            f.read(buf.data(), read_size);
        }
    });

    double wiseio_ms = Measure(warmup, iters, [&]() {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        std::vector<uint8_t> buf(read_size);
        for (size_t off : offsets) {
            stream.CustomRead(buf, off);
        }
    });

    return {
        {"random_access", "fstream", fstream_ms, MBps(total, fstream_ms)},
        {"random_access", "WiseIO",  wiseio_ms,  MBps(total, wiseio_ms)}
    };
}

// ==================== main ====================

int main() {
    InitColors();

    logging::Logger::SetupLogger(
        logging::LoggerMode::kError,
        logging::LoggerIOMode::kSync,
        false);

    fs::path tmp = fs::temp_directory_path() / "wiseio_bench";
    fs::path cache = fs::temp_directory_path() / "wiseio_bench_cache";
    fs::create_directories(tmp);
    fs::create_directories(cache);
    wiseio::Storage::SetCacheDir(cache.string());

    constexpr size_t FILE_16MB  = 16 * 1024 * 1024;
    constexpr size_t FILE_64MB  = 64 * 1024 * 1024;
    constexpr int    WARMUP     = 2;
    constexpr int    ITERS      = 7;
    constexpr int    CHUNKS     = 50;
    constexpr size_t CHUNK_PAY  = 64 * 1024;
    constexpr int    RAND_READS = 200;
    constexpr size_t RAND_SIZE  = 4096;

    PrintHeader();

    std::cout << BOLD << "  Config:\n" << RESET;
    std::cout << "    Sequential  file size : " << (FILE_64MB / 1024 / 1024) << " MB\n";
    std::cout << "    Chunked     chunks    : " << CHUNKS << " × " << (CHUNK_PAY / 1024) << " KB payload\n";
    std::cout << "    Random      reads     : " << RAND_READS << " × " << RAND_SIZE << " bytes from " << (FILE_16MB / 1024 / 1024) << " MB file\n";
    std::cout << "    Iterations            : " << WARMUP << " warmup + " << ITERS << " measured (median)\n\n";

    {
        PrintSectionHeader("1. Sequential read — читаем весь файл целиком");
        auto raw = MakeRawFile(tmp, "seq.bin", FILE_64MB);
        auto [fs_r, wi_r] = BenchSequential(raw, FILE_64MB, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(wi_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("Sequential", wi_r.mbps, fs_r.mbps);
        std::cout << "\n";
    }

    {
        PrintSectionHeader("2. Chunked read — структурированные чанки [num][len][payload]");
        auto structured = MakeStructuredFile(tmp, "chunked.bin", CHUNKS, CHUNK_PAY);
        auto [fs_r, wi_r] = BenchChunked(structured, CHUNKS, CHUNK_PAY, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(wi_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("Chunked", wi_r.mbps, fs_r.mbps);
        std::cout << "\n";
    }

    {
        PrintSectionHeader("3. Random access — случайные чтения по offset");
        auto raw = MakeRawFile(tmp, "rand.bin", FILE_16MB);
        auto [fs_r, wi_r] = BenchRandomAccess(raw, FILE_16MB, RAND_READS, RAND_SIZE, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(wi_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("Random access", wi_r.mbps, fs_r.mbps);
        std::cout << "\n";
    }

    std::cout << BOLD << CYAN
              << "══════════════════════════════════════════════════════════════════════════\n"
              << RESET;

    fs::remove_all(tmp);
    fs::remove_all(cache);

    return 0;
}
// NOLINTEND