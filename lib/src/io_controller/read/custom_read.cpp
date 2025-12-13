#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <vector>
#include <string>

#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"

using str = std::string;

namespace wiseio {

ssize_t Stream::CustomRead(std::vector<uint8_t>& buffer, size_t offset) {
    if (is_eof_) return 0;

    ssize_t len = CustomRead(buffer.data(), offset, buffer.size());
    if (len >= 0) {
        buffer.resize(len);
    }
    return len;
}


ssize_t Stream::CustomRead(IOBuffer& buffer, size_t offset) {
    if (is_eof_) return 0;

    ssize_t len = CustomRead(buffer.GetDataPtr(), offset, buffer.GetBufferSize());
    if (len >= 0) {
        buffer.SetLen(len);
    }
    return len;
}

ssize_t Stream::CustomRead(str& buffer, size_t offset) {
    if (is_eof_) return 0;

    ssize_t len = CustomRead(reinterpret_cast<uint8_t*>(buffer.data()), offset, buffer.size());
    if (len >= 0) {
        buffer.resize(len);
    }

    return len;
}

} // namespace wiseio

