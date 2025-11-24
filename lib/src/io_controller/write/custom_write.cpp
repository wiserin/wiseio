#include <cstddef>
#include <vector>

#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"


namespace wiseio {

bool Stream::CustomWrite(const std::vector<uint8_t>& buffer, size_t offset) {
    if (mode_ != OpenMode::kWrite || mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = CustomWrite(buffer.data(), offset, buffer.size());
    return state;
}


bool Stream::CustomWrite(const IOBuffer& buffer, size_t offset) {
    if (mode_ != OpenMode::kWrite || mode_ != OpenMode::kReadAndWrite) {
        logger_.Exception("Для использования этого метода файл должен быть открыт в режиме Write");
        return false;
    }
    bool state = CustomWrite(buffer.GetDataPtr(), offset, buffer.GetBufferLen());
    return state;
}

} // namespace wiseio
