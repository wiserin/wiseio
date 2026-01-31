#include <cstddef>  // Copyright 2025 wiserin
#include <stdexcept>
#include <utility>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <string>

#include "wise-io/byte/bytefile.hpp"
#include "wise-io/byte/chunks.hpp"
#include "wise-io/stream.hpp"


using str = std::string;

namespace wiseio {


ByteFile::ByteFile(const char* file_name)
        : file_engine_(ByteFileEngine(file_name)) {}


void ByteFile::AddChunk(std::unique_ptr<BaseChunk> chunk, str&& name) {
    if (IsNameInIndex(name)) {
        throw std::logic_error("Имя уже занято");
    }
    index_[name] = chunk.get();
    layout_.push_back(std::move(chunk));
}


BaseChunk& ByteFile::GetChunk(const str& name) {
    if (!IsNameInIndex(name)) {
        throw std::logic_error("Чанк с таким именем не найден");
    }
    return *index_[name];
}


BaseChunk& ByteFile::GetAndLoadChunk(const str& name) {
    if (!IsNameInIndex(name)) {
        throw std::logic_error("Чанк с таким именем не найден");
    }
    BaseChunk& chunk = *index_[name];
    file_engine_.ReadChunk(chunk);
    return chunk;
}


bool ByteFile::IsNameInIndex(const str& name) {
    return index_.contains(name);
}


void ByteFile::InitChunksFromFile() {
    file_engine_.InitChunks(layout_);
}


void ByteFile::Compile() {
    file_engine_.CompileFile(layout_);
}


} // namespace wiseio
