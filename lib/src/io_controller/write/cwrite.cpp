#include <vector>  // Copyright 2025 wiserin

#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"


namespace wiseio {

bool Stream::CWrite(const std::vector<uint8_t>& buffer) {
    if (mode_ != OpenMode::kAppend && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = CWrite(buffer.data(), buffer.size());
    return state;
}


bool Stream::CWrite(const IOBuffer& buffer) {
    if (mode_ != OpenMode::kWrite && mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = CWrite(buffer.GetDataPtr(), buffer.GetBufferLen());
    return state;
}

} // namespace wiseio
