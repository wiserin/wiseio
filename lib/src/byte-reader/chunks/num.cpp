#include <cstddef>  // Copyright 2025 wiserin
#include <string>
#include <sys/types.h>
#include <vector>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/byte/storage.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"


using str = std::string;

namespace wiseio {


NumChunk::NumChunk(NumSize size)
        : size_(size) {}


void NumChunk::Init(Stream& stream) {
    offset_ = stream.GetCursor();
    stream.SetCursor(offset_ + static_cast<int>(size_));
    state_ = ChunkInitState::kFileBacked;
}


void NumChunk::Load(Stream& stream) {
    if (!IsInitialized()) {
        throw std::runtime_error("Для загрузки чанк должен быть инициализирован");
    }
    std::vector<uint8_t>& data = data_.GetData();
    data.resize(static_cast<int>(size_));
    stream.CustomRead(data, offset_);
}


std::vector<uint8_t> NumChunk::GetCompiledChunk() {
    return data_.GetData();
} 


bool NumChunk::IsInitialized() {
    return state_ == ChunkInitState::kFileBacked;
}


uint64_t NumChunk::GetOffset() {
    return offset_;
}


uint64_t NumChunk::GetSize() {
    return static_cast<size_t>(size_);
}


Storage& NumChunk::GetStorage() {
    return data_;
}


} // namespace wiseio
