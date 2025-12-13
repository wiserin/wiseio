#include <cstdint>  // Copyright 2025 wiserin
#include <vector>
#include <string>

#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/buffer.hpp"


using str = std::string;

namespace wiseio {

bool Stream::AWrite(const std::vector<uint8_t>& buffer) {
    if (mode_ != OpenMode::kAppend) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Append");
        return false;
    }
    bool state = AWrite(buffer.data(), buffer.size());
    return state;
}


bool Stream::AWrite(const IOBuffer& buffer) {
    if (mode_ != OpenMode::kAppend) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Append");
        return false;
    }
    bool state = AWrite(buffer.GetDataPtr(), buffer.GetBufferLen());
    return state;
}


bool Stream::AWrite(const str& buffer) {
    if (mode_ != OpenMode::kAppend) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Append");
        return false;
    }
    bool state = AWrite(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());
    return state;
}

} // namespace wiseio
