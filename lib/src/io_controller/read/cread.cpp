#include <cstdint>  // Copyright 2025 wiserin
#include <string>
#include <vector>

#include <core.h>

#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"


using str = std::string;

namespace wiseio {

ssize_t Stream::CRead(std::vector<uint8_t>& buffer) {
    if (is_eof_) {
        return 0;
    }
    FdCheck();
    if (mode_ != OpenMode::kRead && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме read");
        return 0;
    }

    ssize_t len =  wcore_cread(
        fd_, buffer.data(), buffer.size(),
        &is_eof_, &cursor_);
    if (len >= 0) {
        buffer.resize(len);
    }
    return len;
}


ssize_t Stream::CRead(IOBuffer& buffer) {
    if (is_eof_) {
        return 0;
    }
    FdCheck();
    if (mode_ != OpenMode::kRead && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме read");
        return 0;
    }

    ssize_t len =  wcore_cread(
        fd_, buffer.GetDataPtr(), buffer.GetBufferSize(),
        &is_eof_, &cursor_);
    if (len >= 0) {
        buffer.ResizeBuffer(len);
    }
    return len;
}

ssize_t Stream::CRead(str& buffer) {
    if (is_eof_) {
        return 0;
    }
    FdCheck();
    if (mode_ != OpenMode::kRead && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме read");
        return 0;
    }

    ssize_t len =  wcore_cread(
        fd_, reinterpret_cast<uint8_t*>(buffer.data()), buffer.size(),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        &is_eof_, &cursor_);
    if (len >= 0) {
        buffer.resize(len);
    }
    return len;
}

} // namespace wiseio

