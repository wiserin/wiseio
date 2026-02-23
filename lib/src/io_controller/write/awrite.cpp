#include <cstdint>  // Copyright 2025 wiserin
#include <string>
#include <vector>

#include <core.h>

#include "wise-io/buffer.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"


using str = std::string;

namespace wiseio {

bool Stream::AWrite(const std::vector<uint8_t>& buffer) {
    if (mode_ != OpenMode::kAppend) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Append");
        return false;
    }
    bool state = wcore_awrite(
        fd_, buffer.data(), buffer.size());
    return state;
}


bool Stream::AWrite(const IOBuffer& buffer) {
    if (mode_ != OpenMode::kAppend) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Append");
        return false;
    }
    bool state = wcore_awrite(
        fd_, buffer.GetDataPtr(), buffer.GetBufferSize());
    return state;
}


bool Stream::AWrite(const str& buffer) {
    if (mode_ != OpenMode::kAppend) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Append");
        return false;
    }
    bool state = wcore_awrite(
        fd_, reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    return state;
}

} // namespace wiseio
