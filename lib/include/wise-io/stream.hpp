#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "logging/logger.hpp"
#include "wise-io/schemas.hpp"

using stat_t = struct stat;
using str = std::string;


namespace wiseio {

class IOBuffer;

class Stream {
    int fd_ = -1;
    bool is_eof_ = false;
    OpenMode mode_;

    size_t cursor_ = 0;

    logging::Logger logger_;

    bool ORead(const char* path);
    bool OWrite(const char* path);
    bool OAppend(const char* path);
    bool ReadAndWrite(const char* path);

    bool Open(const char* path);

    ssize_t CRead(uint8_t* buffer, size_t buffer_size);
    ssize_t CustomRead(uint8_t* buffer, size_t offset, size_t buffer_size);

    bool AWrite(const uint8_t* buffer, size_t buffer_size);
    bool CWrite(const uint8_t* buffer, size_t buffer_size);
    bool CustomWrite(const uint8_t* buffer, size_t offset, size_t buffer_size);

    void UpdateStat(stat_t& file_stat) const;
    void FdCheck() const;

    Stream(OpenMode mode);

 public:
    Stream() = default;
    Stream(Stream&& stream);
    Stream& operator=(Stream&& stream);

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
    bool CustomWrite(const std::vector<uint8_t>& buffer, size_t offset);
    bool CustomWrite(const IOBuffer& buffer, size_t offset);
    bool CustomWrite(const str& buffer, size_t offset);

    void SetCursor(size_t position);

    bool IsEOF() const;
    size_t GetFileSize() const;

    void Close();

    friend Stream CreateStream(const char* name, OpenMode mode);

    ~Stream();
};


Stream CreateStream(const char* name, OpenMode mode);


} // namespace wiseio
