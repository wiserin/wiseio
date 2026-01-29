#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
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
    OpenMode mode_;

    size_t cursor_ = 0;

    logging::Logger logger_;

    bool Open(const char* path);

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
    bool CustomWrite(const std::vector<uint8_t>& buffer, size_t offset) const;
    bool CustomWrite(const IOBuffer& buffer, size_t offset) const;
    bool CustomWrite(const str& buffer, size_t offset) const;

    void SetCursor(size_t position);

    size_t GetCursor();

    bool IsEOF() const;
    size_t GetFileSize() const;

    void Close();

    friend Stream CreateStream(const char* name, OpenMode mode);

    ~Stream();
};


Stream CreateStream(const char* name, OpenMode mode);


} // namespace wiseio
