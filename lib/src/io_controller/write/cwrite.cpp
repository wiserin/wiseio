#include <cstdint>  // Copyright 2025 wiserin
#include <string>
#include <vector>

#include <core.h>

#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"
#include "wise-io/schemas.hpp"


using str = std::string;

namespace wiseio {

bool Stream::CWrite(const std::vector<uint8_t>& buffer) {
    if (mode_ != OpenMode::kWrite && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = wcore_cwrite(
        fd_, buffer.data(), buffer.size(),
        &cursor_);
    return state;
}


bool Stream::CWrite(const IOBuffer& buffer) {
    if (mode_ != OpenMode::kWrite && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = wcore_cwrite(
        fd_, buffer.GetDataPtr(), buffer.GetBufferSize(),
        &cursor_);
    return state;
}

bool Stream::CWrite(const str& buffer) {
    if (mode_ != OpenMode::kWrite && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = wcore_cwrite(
        fd_, reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size(),  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        &cursor_);
    return state;
}


} // namespace wiseio
