#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <vector>

#include "wise-io/stream.hpp"
#include "logging/logger.hpp"


namespace wiseio {

ssize_t Stream::CustomRead(std::vector<uint8_t>& buffer, size_t offset, size_t buffer_size) {
    if (is_eof_) {
        return 0;
    } else if (buffer.size() < buffer_size) {
        buffer.resize(buffer_size);
    }
    ssize_t len = CustomRead(buffer.data(), offset, buffer_size);
    if (len >= 0) {
        buffer.resize(len);
    }
    return len;
}


ssize_t Stream::CustomRead(IOBuffer& buffer, size_t offset, size_t buffer_size) {
    if (is_eof_) {
        return 0;
    } else if (buffer.GetBufferSize() < buffer_size_) {
        logger_.Exception("Размер временного буфера не может быть меньше заданного");
    }
    ssize_t len = CustomRead(buffer.GetDataPtr(), offset, buffer_size);
    if (len >= 0) {
        buffer.SetLen(len);
    }
    return len;
}

} // namespace wiseio

