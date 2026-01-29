#include <cstddef>  // Copyright 2025 wiserin
#include <stdexcept>
#include <utility>
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <string>

#include "wise-io/byte-reader.hpp"
#include "wise-io/stream.hpp"


using str = std::string;

namespace wiseio {


ByteFile::ByteFile(const char* file_name)
        : file_engine_(ByteFileEngine(file_name)) {}


void ByteFile::AddChunk(std::unique_ptr<BaseChunk> chunk, str&& name) {
    if (IsNameInIndex(name)) {
        throw std::logic_error("Имя уже занято");
    }
    layout_.push_back(std::move(chunk));
    index_[name] = *chunk;
}


BaseChunk& ByteFile::GetChunk(const str& name) {
    if (!IsNameInIndex(name)) {
        throw std::logic_error("Чанк с таким именем не найден");
    }
    return index_[name];
}


bool ByteFile::IsNameInIndex(const str& name) {
    return index_.contains(name);
}


void ByteFile::ReadChunksFromFile() {
    file_engine_.InitChunks(layout_);
}


} // namespace wiseio
