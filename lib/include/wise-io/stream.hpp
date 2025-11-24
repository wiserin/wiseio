#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "logging/logger.hpp"
#include "wise-io/schemas.hpp"


namespace wiseio {

class IOBuffer {
    std::unique_ptr<uint8_t[]> data_;
    uint64_t buffer_size_;
    size_t len_;
    size_t cursor_;

 public:
    IOBuffer(size_t buffer_size = 4096);

    IOBuffer(IOBuffer&& buffer);

    size_t ReadFromBuffer() const;
    bool AddByte(uint8_t byte);
    bool Add(const std::vector<uint8_t>& buffer);
    bool Clear();

    size_t GetBufferLen() const;
    size_t GetBufferSize() const;
    uint8_t* GetDataPtr() const;

    bool SetLen(size_t len);

    bool SetCursor(size_t position);


    ~IOBuffer() = default;
};


class Stream {
    int fd_;
    uint64_t buffer_size_;
    bool is_eof_ = false;
    OpenMode mode_;

    size_t cursor_;

    logging::Logger logger_;

    bool ORead(const char* path);
    bool OWrite(const char* path);
    bool OAppend(const char* path);
    bool ReadAndWrite(const char* path);

    bool Open(const char* path, OpenMode mode);

    ssize_t Read(uint8_t* buffer, size_t offset);
    ssize_t CRead(uint8_t* buffer);
    ssize_t CustomRead(uint8_t* buffer, size_t offset, size_t buffer_size);

    bool AWrite(const uint8_t* buffer, size_t buffer_size);
    bool CustomWrite(const uint8_t* buffer, size_t offset, size_t buffer_size);

 public:
    Stream(
        const char* file_path,
        OpenMode mode,
        uint64_t buffer_size = 4096);
    

    ssize_t Read(std::vector<uint8_t>& buffer, size_t offset = 0);
    ssize_t Read(IOBuffer& buffer, size_t offset = 0);
    ssize_t CRead(std::vector<uint8_t>& buffer);
    ssize_t CRead(IOBuffer& buffer);
    ssize_t CustomRead(std::vector<uint8_t>& buffer, size_t offset, size_t buffer_size);
    ssize_t CustomRead(IOBuffer& buffer, size_t offset, size_t buffer_size);

    bool AWrite(const std::vector<uint8_t>& buffer);
    bool AWrite(const IOBuffer& buffer);
    bool CustomWrite(const std::vector<uint8_t>& buffer, size_t offset);
    bool CustomWrite(const IOBuffer& buffer, size_t offset);

    ~Stream();
};


} // namespace wiseio
