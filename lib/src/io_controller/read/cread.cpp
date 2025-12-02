#include <cstdint>  // Copyright 2025 wiserin
#include <vector>

#include "wise-io/stream.hpp"
#include "logging/logger.hpp"


namespace wiseio {

ssize_t Stream::CRead(std::vector<uint8_t>& buffer) {
    if (is_eof_) {
        return 0;
    } else if (buffer.size() < buffer_size_) {
        buffer.resize(buffer_size_);
    }
    ssize_t len =  CRead(buffer.data());
    if (len >= 0) {
        buffer.resize(len);
    }
    return len;
}


ssize_t Stream::CRead(IOBuffer& buffer) {
    if (is_eof_) {
        return 0;
    } else if (buffer.GetBufferSize() < buffer_size_) {
        logger_.Exception("Размер временного буфера не может быть меньше заданного");
    }
    ssize_t len =  CRead(buffer.GetDataPtr());
    if (len >= 0) {
        buffer.SetLen(len);
    }
    return len;
}

} // namespace wiseio

