// perf/bench_stream.cpp
// Чистый бенч интерфейса Stream: CRead / ReadAll / CWrite / CursorRead
// Сравниваем каждый метод Stream vs прямой аналог fstream/syscall
//
// Сценарии:
//   1. CRead (буферизованное sequential чтение чанками)
//   2. ReadAll (весь файл за один вызов)
//   3. CustomRead (pread по offset без seek)
//   4. CWrite (буферизованная запись чанками)
//   5. Открытие/закрытие файла (overhead на создание Stream)
// NOLINTBEGIN
#include <algorithm>
#include <chrono>
#include <cstdint>
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
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;
using Ms    = std::chrono::duration<double, std::milli>;

// =====================================================================
// ANSI (CI-safe)
// =====================================================================

static const char* GREEN  = "";
static const char* YELLOW = "";
static const char* CYAN   = "";
static const char* BLUE   = "";
static const char* RED    = "";
static const char* BOLD   = "";
static const char* DIM    = "";
static const char* RESET  = "";

static void InitColors() {
    bool ci       = std::getenv("CI") != nullptr;
    bool no_color = std::getenv("NO_COLOR") != nullptr;
    if (!ci && !no_color) {
        GREEN  = "\033[32m";
        YELLOW = "\033[33m";
        CYAN   = "\033[36m";
        BLUE   = "\033[34m";
        RED    = "\033[31m";
        BOLD   = "\033[1m";
        DIM    = "\033[2m";
        RESET  = "\033[0m";
    }
}

// =====================================================================
// Измерение
// =====================================================================

static double Measure(int warmup, int iters, std::function<void()> fn) {
    for (int i = 0; i < warmup; ++i) fn();
    std::vector<double> t;
    t.reserve(iters);
    for (int i = 0; i < iters; ++i) {
        auto t0 = Clock::now();
        fn();
        t.push_back(Ms(Clock::now() - t0).count());
    }
    std::sort(t.begin(), t.end());
    return t[t.size() / 2];
}

static double MBps(size_t bytes, double ms) {
    return (static_cast<double>(bytes) / (1024.0 * 1024.0)) / (ms / 1000.0);
}

// =====================================================================
// Генерация файлов
// =====================================================================

static std::string MakeRawFile(const fs::path& dir, const std::string& name, size_t sz) {
    auto path = dir / name;
    std::ofstream f(path, std::ios::binary);
    std::mt19937 rng(0xDEAD);
    std::uniform_int_distribution<int> d(0, 255);
    std::vector<uint8_t> buf(65536);
    size_t done = 0;
    while (done < sz) {
        size_t n = std::min(buf.size(), sz - done);
        for (size_t i = 0; i < n; ++i) buf[i] = static_cast<uint8_t>(d(rng));
        f.write(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(n));
        done += n;
    }
    return path.string();
}

// =====================================================================
// Структуры результатов
// =====================================================================

struct Row {
    std::string method;
    std::string impl;
    double      ms;
    double      mbps;
    size_t      bytes;
};

// =====================================================================
// Отрисовка
// =====================================================================

static std::string Bar(double ratio, int w = 24) {
    // ratio = Stream_mbps / fstream_mbps, cap 2x → full bar
    int n = static_cast<int>(std::min(ratio, 2.0) / 2.0 * w);
    std::string s = "[";
    for (int i = 0; i < w; ++i) s += (i < n ? "█" : "░");
    return s + "]";
}

static void PrintHeader() {
    std::cout << "\n" << BOLD << BLUE;
    std::cout << "╔══════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║           WiseIO Stream  vs  fstream  —  Raw I/O Benchmark             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
}

static void PrintSectionHeader(const std::string& title) {
    std::cout << BOLD << YELLOW << "  ┌─ " << title << RESET << "\n";
}

static void PrintSectionFooter() {
    std::cout << "  └\n\n";
}

static void PrintTableHeader() {
    std::cout << BOLD
              << "  │  " << std::left
              << std::setw(12) << "Impl"
              << std::setw(12) << "Median ms"
              << std::setw(12) << "MB/s"
              << std::setw(10) << "Bytes"
              << "Ratio vs fstream\n"
              << "  │  " << std::string(64, '-') << "\n"
              << RESET;
}

static void PrintRow(const Row& r, double baseline_mbps) {
    bool is_stream = (r.impl == "Stream");
    double ratio   = r.mbps / baseline_mbps;

    std::cout << "  │  ";

    if (is_stream) std::cout << CYAN << BOLD;
    std::cout << std::left << std::setw(12) << r.impl << RESET;

    std::cout << DIM << std::setw(12) << std::fixed << std::setprecision(3) << r.ms << RESET;

    if (is_stream) std::cout << (ratio >= 1.0 ? GREEN : RED) << BOLD;
    std::cout << std::setw(12) << std::fixed << std::setprecision(1) << r.mbps;
    if (is_stream) std::cout << RESET;

    std::cout << DIM << std::setw(10) << r.bytes << RESET;

    if (is_stream) {
        std::cout << (ratio >= 1.0 ? GREEN : RED)
                  << Bar(ratio) << " "
                  << std::fixed << std::setprecision(2) << ratio << "x"
                  << RESET;
    } else {
        std::cout << DIM << "baseline" << RESET;
    }
    std::cout << "\n";
}

static void PrintSummary(const std::string& method, double stream_mbps, double fs_mbps) {
    double r     = stream_mbps / fs_mbps;
    bool faster  = r >= 1.0;
    std::cout << "  "
              << (faster ? GREEN : RED) << BOLD
              << (faster ? "  ✓" : "  ✗") << " [" << method << "]  Stream is "
              << std::fixed << std::setprecision(2)
              << (faster ? r : 1.0 / r) << "x "
              << (faster ? "faster" : "slower") << " than fstream"
              << RESET << "\n";
}

// =====================================================================
// Бенчмарки
// =====================================================================

static std::pair<Row, Row> BenchCRead(
        const std::string& path, size_t file_sz, size_t chunk_sz,
        int warmup, int iters) {


    double fs_ms = Measure(warmup, iters, [&]() {
        std::ifstream f(path, std::ios::binary);
        std::vector<char> buf(chunk_sz);
        while (f) f.read(buf.data(), static_cast<std::streamsize>(chunk_sz));
    });

    double st_ms = Measure(warmup, iters, [&]() {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        std::vector<uint8_t> buf(chunk_sz);
        while (!stream.IsEOF()) stream.CRead(buf);
    });

    return {
        {"CRead", "fstream", fs_ms, MBps(file_sz, fs_ms), file_sz},
        {"CRead", "Stream",  st_ms, MBps(file_sz, st_ms), file_sz}
    };
}

static std::pair<Row, Row> BenchReadAll(
        const std::string& path, size_t file_sz,
        int warmup, int iters) {

    double fs_ms = Measure(warmup, iters, [&]() {
        std::ifstream f(path, std::ios::binary);
        std::vector<char> buf(file_sz);
        f.read(buf.data(), static_cast<std::streamsize>(file_sz));
    });

    double st_ms = Measure(warmup, iters, [&]() {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        std::vector<uint8_t> buf;
        stream.ReadAll(buf);
    });

    return {
        {"ReadAll", "fstream", fs_ms, MBps(file_sz, fs_ms), file_sz},
        {"ReadAll", "Stream",  st_ms, MBps(file_sz, st_ms), file_sz}
    };
}

static std::pair<Row, Row> BenchCustomRead(
        const std::string& path, size_t file_sz, int n_reads, size_t read_sz,
        int warmup, int iters) {

    std::mt19937 rng(0xBEEF);
    std::uniform_int_distribution<size_t> dist(0, file_sz - read_sz - 1);
    std::vector<size_t> offsets(n_reads);
    for (auto& o : offsets) o = dist(rng);

    size_t total = static_cast<size_t>(n_reads) * read_sz;

    double fs_ms = Measure(warmup, iters, [&]() {
        std::ifstream f(path, std::ios::binary);
        std::vector<char> buf(read_sz);
        for (size_t off : offsets) {
            f.seekg(static_cast<std::streamoff>(off));
            f.read(buf.data(), static_cast<std::streamsize>(read_sz));
        }
    });

    double st_ms = Measure(warmup, iters, [&]() {
        auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
        std::vector<uint8_t> buf(read_sz);
        for (size_t off : offsets) {
            stream.CustomRead(buf, off);
        }
    });

    return {
        {"CustomRead", "fstream", fs_ms, MBps(total, fs_ms), total},
        {"CustomRead", "Stream",  st_ms, MBps(total, st_ms), total}
    };
}

static std::pair<Row, Row> BenchCWrite(
        const fs::path& tmp_dir, size_t total_sz, size_t chunk_sz,
        int warmup, int iters) {

    std::vector<uint8_t> payload(chunk_sz, 0xAB);
    int n_chunks = static_cast<int>(total_sz / chunk_sz);

    auto fs_path = (tmp_dir / "write_fs.bin").string();
    auto st_path = (tmp_dir / "write_st.bin").string();

    double fs_ms = Measure(warmup, iters, [&]() {
        std::ofstream f(fs_path, std::ios::binary | std::ios::trunc);
        for (int i = 0; i < n_chunks; ++i) {
            f.write(reinterpret_cast<char*>(payload.data()),
                    static_cast<std::streamsize>(chunk_sz));
        }
    });

    double st_ms = Measure(warmup, iters, [&]() {
        auto stream = wiseio::CreateStream(st_path.c_str(), wiseio::OpenMode::kWrite);
        for (int i = 0; i < n_chunks; ++i) {
            stream.CWrite(payload);
        }
    });

    return {
        {"CWrite", "fstream", fs_ms, MBps(total_sz, fs_ms), total_sz},
        {"CWrite", "Stream",  st_ms, MBps(total_sz, st_ms), total_sz}
    };
}

static std::pair<Row, Row> BenchOpenClose(
        const std::string& path, int n_opens,
        int warmup, int iters) {

    size_t fake_bytes = static_cast<size_t>(n_opens) * 1;

    double fs_ms = Measure(warmup, iters, [&]() {
        for (int i = 0; i < n_opens; ++i) {
            std::ifstream f(path, std::ios::binary);
            (void)f.is_open();
        }
    });

    double st_ms = Measure(warmup, iters, [&]() {
        for (int i = 0; i < n_opens; ++i) {
            auto stream = wiseio::CreateStream(path.c_str(), wiseio::OpenMode::kRead);
            (void)stream.IsOpen();
        }
    });

    double fs_us_per_op = (fs_ms * 1000.0) / n_opens;
    double st_us_per_op = (st_ms * 1000.0) / n_opens;

    return {
        {"Open/Close", "fstream", fs_ms, fs_us_per_op, fake_bytes},
        {"Open/Close", "Stream",  st_ms, st_us_per_op, fake_bytes}
    };
}

// =====================================================================
// main
// =====================================================================

int main() {
    InitColors();

    logging::Logger::SetupLogger(
        logging::LoggerMode::kError,
        logging::LoggerIOMode::kSync,
        false);

    fs::path tmp   = fs::temp_directory_path() / "wiseio_stream_bench";
    fs::path cache = fs::temp_directory_path() / "wiseio_stream_cache";
    fs::create_directories(tmp);
    fs::create_directories(cache);
    wiseio::Storage::SetCacheDir(cache.string());

    constexpr size_t FILE_32MB   = 32ULL * 1024 * 1024;
    constexpr size_t FILE_256MB  = 256ULL * 1024 * 1024;
    constexpr size_t CHUNK_4K    = 4  * 1024;
    constexpr size_t CHUNK_64K   = 64 * 1024;
    constexpr size_t CHUNK_1M    = 1  * 1024 * 1024;
    constexpr int    WARMUP      = 2;
    constexpr int    ITERS       = 7;
    constexpr int    RAND_READS  = 500;
    constexpr size_t RAND_SZ     = 4096;
    constexpr int    OPENS       = 500;

    PrintHeader();

    std::cout << BOLD << "  Config:\n" << RESET;
    std::cout << "    ReadAll/CRead file   : " << (FILE_256MB / 1024 / 1024) << " MB\n";
    std::cout << "    CustomRead file      : " << (FILE_32MB  / 1024 / 1024) << " MB,  "
              << RAND_READS << " reads × " << RAND_SZ << " B\n";
    std::cout << "    CRead chunk sizes    : "
              << (CHUNK_4K / 1024) << " KB / "
              << (CHUNK_64K / 1024) << " KB / "
              << (CHUNK_1M / 1024 / 1024) << " MB\n";
    std::cout << "    CWrite total         : " << (FILE_32MB / 1024 / 1024) << " MB\n";
    std::cout << "    Open/Close           : " << OPENS << " ops\n";
    std::cout << "    Iterations           : " << WARMUP << " warmup + "
              << ITERS << " measured (median)\n\n";

    std::cout << DIM << "  [Preparing test files...]\n" << RESET;
    auto big_file  = MakeRawFile(tmp, "big.bin",   FILE_256MB);
    auto rand_file = MakeRawFile(tmp, "rand.bin",  FILE_32MB);
    std::cout << DIM << "  [Done. Running benchmarks...]\n\n" << RESET;

    struct Summary { std::string method; double stream_mbps; double fs_mbps; };
    std::vector<Summary> summaries;

    {
        PrintSectionHeader("1. ReadAll  —  весь файл за один вызов");
        auto [fs_r, st_r] = BenchReadAll(big_file, FILE_256MB, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(st_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("ReadAll", st_r.mbps, fs_r.mbps);
        summaries.push_back({"ReadAll", st_r.mbps, fs_r.mbps});
        std::cout << "\n";
    }

    {
        PrintSectionHeader("2. CRead 4 KB chunks  —  буферизованное последовательное чтение");
        auto [fs_r, st_r] = BenchCRead(big_file, FILE_256MB, CHUNK_4K, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(st_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("CRead/4K", st_r.mbps, fs_r.mbps);
        summaries.push_back({"CRead/4K", st_r.mbps, fs_r.mbps});
        std::cout << "\n";
    }

    {
        PrintSectionHeader("3. CRead 64 KB chunks");
        auto [fs_r, st_r] = BenchCRead(big_file, FILE_256MB, CHUNK_64K, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(st_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("CRead/64K", st_r.mbps, fs_r.mbps);
        summaries.push_back({"CRead/64K", st_r.mbps, fs_r.mbps});
        std::cout << "\n";
    }

    {
        PrintSectionHeader("4. CRead 1 MB chunks");
        auto [fs_r, st_r] = BenchCRead(big_file, FILE_256MB, CHUNK_1M, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(st_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("CRead/1M", st_r.mbps, fs_r.mbps);
        summaries.push_back({"CRead/1M", st_r.mbps, fs_r.mbps});
        std::cout << "\n";
    }

    {
        PrintSectionHeader("5. CustomRead  —  случайные чтения (pread vs seekg+read)");
        auto [fs_r, st_r] = BenchCustomRead(rand_file, FILE_32MB, RAND_READS, RAND_SZ, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(st_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("CustomRead", st_r.mbps, fs_r.mbps);
        summaries.push_back({"CustomRead", st_r.mbps, fs_r.mbps});
        std::cout << "\n";
    }

    {
        PrintSectionHeader("6. CWrite  —  последовательная запись 64 KB чанками");
        auto [fs_r, st_r] = BenchCWrite(tmp, FILE_32MB, CHUNK_64K, WARMUP, ITERS);
        PrintTableHeader();
        PrintRow(fs_r, fs_r.mbps);
        PrintRow(st_r, fs_r.mbps);
        PrintSectionFooter();
        PrintSummary("CWrite", st_r.mbps, fs_r.mbps);
        summaries.push_back({"CWrite", st_r.mbps, fs_r.mbps});
        std::cout << "\n";
    }

    {
        PrintSectionHeader("7. Open / Close  —  overhead создания Stream (µs/op)");
        auto [fs_r, st_r] = BenchOpenClose(rand_file, OPENS, WARMUP, ITERS);

        std::cout << BOLD
                  << "  │  " << std::left
                  << std::setw(12) << "Impl"
                  << std::setw(12) << "Median ms"
                  << std::setw(12) << "µs/op"
                  << std::setw(10) << "ops"
                  << "Ratio\n"
                  << "  │  " << std::string(64, '-') << "\n"
                  << RESET;

        std::cout << "  │  " << BOLD << std::left << std::setw(12) << "fstream" << RESET
                  << DIM << std::setw(12) << std::fixed << std::setprecision(3) << fs_r.ms << RESET
                  << std::setw(12) << std::fixed << std::setprecision(2) << fs_r.mbps
                  << DIM << std::setw(10) << OPENS << RESET
                  << DIM << "baseline" << RESET << "\n";

        double ratio = fs_r.mbps / st_r.mbps;
        bool better  = st_r.mbps <= fs_r.mbps;
        std::cout << "  │  " << CYAN << BOLD << std::left << std::setw(12) << "Stream" << RESET
                  << DIM << std::setw(12) << std::fixed << std::setprecision(3) << st_r.ms << RESET
                  << (better ? GREEN : RED) << BOLD
                  << std::setw(12) << std::fixed << std::setprecision(2) << st_r.mbps << RESET
                  << DIM << std::setw(10) << OPENS << RESET
                  << (better ? GREEN : RED)
                  << Bar(ratio) << " " << std::fixed << std::setprecision(2) << ratio << "x"
                  << RESET << "\n";

        PrintSectionFooter();
        bool stream_faster = st_r.mbps <= fs_r.mbps;
        double lat_ratio   = stream_faster ? (fs_r.mbps / st_r.mbps) : (st_r.mbps / fs_r.mbps);
        std::cout << "  "
                  << (stream_faster ? GREEN : RED) << BOLD
                  << (stream_faster ? "  ✓" : "  ✗") << " [Open/Close]  Stream is "
                  << std::fixed << std::setprecision(2) << lat_ratio << "x "
                  << (stream_faster ? "lower latency" : "higher latency") << " than fstream"
                  << RESET << "\n\n";
    }

    std::cout << "\n" << BOLD << CYAN
              << "  ┌─ Summary — Stream vs fstream\n"
              << "  │\n" << RESET;

    std::cout << BOLD
              << "  │  " << std::left
              << std::setw(16) << "Scenario"
              << std::setw(14) << "Stream MB/s"
              << std::setw(14) << "fstream MB/s"
              << "Result\n"
              << "  │  " << std::string(60, '-') << "\n"
              << RESET;

    int wins = 0;
    for (const auto& s : summaries) {
        double r      = s.stream_mbps / s.fs_mbps;
        bool faster   = r >= 1.0;
        if (faster) ++wins;

        std::cout << "  │  "
                  << std::left << std::setw(16) << s.method
                  << (faster ? GREEN : RED)
                  << std::setw(14) << std::fixed << std::setprecision(1) << s.stream_mbps
                  << RESET
                  << std::setw(14) << std::fixed << std::setprecision(1) << s.fs_mbps
                  << (faster ? GREEN : RED) << BOLD
                  << (faster ? "▲ " : "▼ ")
                  << std::fixed << std::setprecision(2) << (faster ? r : 1.0/r) << "x"
                  << RESET << "\n";
    }

    std::cout << "  │\n";
    std::cout << "  │  " << BOLD
              << "Score: " << wins << "/" << summaries.size() << " scenarios won by Stream"
              << RESET << "\n";
    std::cout << BOLD << CYAN << "  └\n" << RESET << "\n";

    std::cout << BOLD << BLUE
              << "══════════════════════════════════════════════════════════════════════════\n"
              << RESET;

    fs::remove_all(tmp);
    fs::remove_all(cache);
    return 0;
}
// NOLINTEND