#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <vector>


namespace wiseio {

class IOBuffer {
    std::vector<uint8_t> data_;
    uint64_t buffer_size_;
    size_t len_;
    size_t cursor_;

 public:
    IOBuffer(size_t buffer_size);

    IOBuffer() = default;
    IOBuffer(IOBuffer&& buffer);
    IOBuffer& operator=(IOBuffer&& buffer) = default;

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


} // namespace wiseio