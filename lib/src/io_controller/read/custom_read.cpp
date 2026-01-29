#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <vector>
#include <string>

#include <core.h>

#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"


using str = std::string;

namespace wiseio {

ssize_t Stream::CustomRead(std::vector<uint8_t>& buffer, size_t offset) {
    if (is_eof_) return 0;
    FdCheck();
    if (mode_ != OpenMode::kRead && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме read");
        return false;
    }

    ssize_t len = wcore_custom_read(
        fd_, buffer.data(), offset, buffer.size(),
        &is_eof_);
    if (len >= 0) {
        buffer.resize(len);
    }
    return len;
}


ssize_t Stream::CustomRead(IOBuffer& buffer, size_t offset) {
    if (is_eof_) return 0;
    FdCheck();
    if (mode_ != OpenMode::kRead && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме read");
        return false;
    }

    ssize_t len = wcore_custom_read(
        fd_, buffer.GetDataPtr(), offset, buffer.GetBufferSize(),
        &is_eof_);
    if (len >= 0) {
        buffer.ResizeBuffer(len);
    }
    return len;
}

ssize_t Stream::CustomRead(str& buffer, size_t offset) {
    if (is_eof_) return 0;
    FdCheck();
    if (mode_ != OpenMode::kRead && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме read");
        return false;
    }

    ssize_t len = wcore_custom_read(
        fd_, reinterpret_cast<uint8_t*>(buffer.data()), offset, buffer.size(),
        &is_eof_);
    if (len >= 0) {
        buffer.resize(len);
    }

    return len;
}

} // namespace wiseio

