#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "logging/logger.hpp"
#include "wise-io/schemas.hpp"


using stat_t = struct stat;
using str = std::string;

namespace wiseio {

class IOBuffer;

class Stream {
    int fd_ = -1;
    bool is_eof_ = false;
    OpenMode mode_ = OpenMode::kDefault;
    size_t cursor_ = 0;
    std::filesystem::path file_path_;
    logging::Logger logger_;

    bool Open();

    void FdCheck() const;

    Stream(OpenMode mode, const char* file_name);

 public:
    Stream() = default;
    Stream(Stream&& another) noexcept;
    Stream& operator=(Stream&& another) noexcept;

    Stream(Stream& stream) = delete;
    Stream& operator=(Stream& stream) = delete;

    ssize_t CRead(std::vector<uint8_t>& buffer);
    ssize_t CRead(IOBuffer& buffer);
    ssize_t CRead(str& buffer);
    ssize_t CustomRead(std::vector<uint8_t>& buffer, size_t offset);
    ssize_t CustomRead(IOBuffer& buffer, size_t offset);
    ssize_t CustomRead(str& buffer, size_t offset);
    ssize_t ReadAll(std::vector<uint8_t>& buffer);
    ssize_t ReadAll(IOBuffer& buffer);
    ssize_t ReadAll(str& buffer);

    bool AWrite(const std::vector<uint8_t>& buffer);
    bool AWrite(const IOBuffer& buffer);
    bool AWrite(const str& buffer);
    bool CWrite(const std::vector<uint8_t>& buffer);
    bool CWrite(const IOBuffer& buffer);
    bool CWrite(const str& buffer);
    bool CustomWrite(const std::vector<uint8_t>& buffer, size_t offset) const;  // NOLINT(modernize-use-nodiscard)
    bool CustomWrite(const IOBuffer& buffer, size_t offset) const;  // NOLINT(modernize-use-nodiscard)
    bool CustomWrite(const str& buffer, size_t offset) const;  // NOLINT(modernize-use-nodiscard)

    void SetCursor(size_t position);

    [[nodiscard]] size_t GetCursor() const;
    [[nodiscard]] size_t GetFileSize() const;
    void SetDelete() const;

    [[nodiscard]] bool IsEOF() const;
    [[nodiscard]] bool IsOpen() const;

    void Rename(str&& new_name);
    void Close();

    friend Stream CreateStream(const char* name, OpenMode mode, bool is_temp);
    friend Stream CreateStream(const std::filesystem::path& name, OpenMode mode, bool is_temp);

    ~Stream();
};


[[nodiscard]] Stream CreateStream(const char* name, OpenMode mode, bool is_temp = false);
[[nodiscard]] Stream CreateStream(const std::filesystem::path& name, OpenMode mode, bool is_temp = false);


} // namespace wiseio
