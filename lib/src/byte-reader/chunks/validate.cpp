#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"


using str = std::string;

namespace wiseio {

ValidateChunk::ValidateChunk(
        size_t size,
        std::vector<uint8_t>&& target_value)
    : size_(size)
    , target_value_(std::move(target_value)) {}


void ValidateChunk::Init(wiseio::Stream& stream) {
    std::vector<uint8_t> chunk;
    chunk.resize(size_);
    offset_ = stream.GetCursor();
    stream.CRead(chunk);
    if (!Validate(chunk)) {
        throw std::logic_error("Validate chunk: the data doesn't match");
    }
    state_ = ChunkInitState::kFileBacked;
}


void ValidateChunk::Load(Stream& stream) {
    if (!IsInitialized()) {
        throw std::runtime_error("Для загрузки чанк должен быть инициализирован");
    }
    std::vector<uint8_t>& data = data_.GetData();
    data.resize(static_cast<int>(size_));
    stream.CustomRead(data, offset_);
}


std::vector<uint8_t> ValidateChunk::GetCompiledChunk() {
    return data_.GetData();
}


bool ValidateChunk::IsInitialized() {
    return state_ == ChunkInitState::kFileBacked;
}


uint64_t ValidateChunk::GetOffset() {
    return offset_;
}


uint64_t ValidateChunk::GetSize() {
    return size_;
}


Storage& ValidateChunk::GetStorage() {
    return data_;
}


bool ValidateChunk::Validate(const std::vector<uint8_t>& data) const {
    if (data.size() != target_value_.size()) {
        return false;
    }

    for (int i = 0; i < data.size(); ++i) {
        if (data[i] != target_value_[i]) {
            return false;
        }
    }
    return true;
}


} // namespace wiseio
