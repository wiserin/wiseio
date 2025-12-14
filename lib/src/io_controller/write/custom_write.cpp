#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <string>

#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"


using str = std::string;

namespace wiseio {

bool Stream::CustomWrite(const std::vector<uint8_t>& buffer, size_t offset) {
    if (mode_ != OpenMode::kWrite && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = CustomWrite(buffer.data(), offset, buffer.size());
    return state;
}


bool Stream::CustomWrite(const IOBuffer& buffer, size_t offset) {
    if (mode_ != OpenMode::kWrite && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = CustomWrite(buffer.GetDataPtr(), offset, buffer.GetBufferSize());
    return state;
}

bool Stream::CustomWrite(const str& buffer, size_t offset) {
    if (mode_ != OpenMode::kWrite && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = CustomWrite(reinterpret_cast<const uint8_t*>(buffer.data()), offset, buffer.size());
    return state;
}

} // namespace wiseio
