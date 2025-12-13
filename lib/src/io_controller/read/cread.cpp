#include <cstdint>  // Copyright 2025 wiserin
#include <vector>
#include <string>

#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"

using str = std::string;

namespace wiseio {

ssize_t Stream::CRead(std::vector<uint8_t>& buffer) {
    if (is_eof_) return 0;

    ssize_t len =  CRead(buffer.data(), buffer.size());
    if (len >= 0) {
        buffer.resize(len);
    }
    return len;
}


ssize_t Stream::CRead(IOBuffer& buffer) {
    if (is_eof_) return 0;

    ssize_t len =  CRead(buffer.GetDataPtr(), buffer.GetBufferSize());
    if (len >= 0) {
        buffer.SetLen(len);
    }
    return len;
}

ssize_t Stream::CRead(str& buffer) {
    if (is_eof_) return 0;

    ssize_t len =  CRead(reinterpret_cast<uint8_t*>(buffer.data()), buffer.size());
    if (len >= 0) {
        buffer.resize(len);
    }
    return len;
}

} // namespace wiseio

