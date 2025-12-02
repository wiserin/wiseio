#include <vector>  // Copyright 2025 wiserin

#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"


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

} // namespace wiseio
