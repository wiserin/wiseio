#include <cstdint>
#include <memory>
#include <utility>

#include <vector>

#include "wise-io/stream.hpp"


namespace wiseio {

IOBuffer::IOBuffer(size_t buffer_size)
        : buffer_size_(buffer_size) {
    data_ = std::make_unique<uint8_t[]>(buffer_size);
    len_ = 0;
    cursor_ = 0;
}


IOBuffer::IOBuffer(IOBuffer&& buffer) {
    data_ = std::move(buffer.data_);
    buffer_size_ = buffer.buffer_size_;
    len_ = buffer.len_;
    cursor_ = buffer.cursor_;
}


size_t IOBuffer::ReadFromBuffer() const {
    return 1;
}


bool IOBuffer::AddByte(uint8_t byte) {
    if (len_ == buffer_size_) {
        return false;
    }
    data_[len_] = byte;
    ++len_;
    return true;
}


bool IOBuffer::Add(const std::vector<uint8_t>& buffer) {
    if(len_ + buffer.size() >= buffer_size_) {
        return false;
    }

    for (uint8_t el : buffer) {
        data_[len_] = el;
        ++len_;
    }
    return true;
}


bool IOBuffer::Clear() {
    data_.reset();
    data_ = std::make_unique<uint8_t[]>(buffer_size_);
    len_ = 0;
    cursor_ = 0;
    return true;
}


size_t IOBuffer::GetBufferLen() const {
    return len_;
}


size_t IOBuffer::GetBufferSize() const {
    return buffer_size_;
}


uint8_t* IOBuffer::GetDataPtr() const {
    return data_.get();
}


bool IOBuffer::SetLen(size_t len) {
    if (len <= buffer_size_) {
        len_ = len;
        return true;
    } else {
        return false;
    }
}


bool IOBuffer::SetCursor(size_t position) {
    if (position < buffer_size_) {
        cursor_ = position;
        return true;
    } else {
        return false;
    }
}

} // namespase wiseio
