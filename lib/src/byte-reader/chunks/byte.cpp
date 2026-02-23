#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/byte/storage.hpp"
#include "wise-io/byte/views.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"


using str = std::string;

namespace wiseio {

ByteChunk::ByteChunk(NumSize size, Endianess num_endianess)
    : len_num_size_(size)
    , num_endianess_(num_endianess) {}


void ByteChunk::Init(wiseio::Stream& stream) {
    std::vector<uint8_t> num;
    num.resize(static_cast<int>(len_num_size_));
    stream.CRead(num);
    offset_ = stream.GetCursor();
    NumView view(num, num_endianess_);
    SetSizeNum(view);
    stream.SetCursor(offset_ + size_);
    state_ = ChunkInitState::kFileBacked;
}


void ByteChunk::Load(Stream& stream) {
    if (!IsInitialized()) {
        throw std::runtime_error("Для загрузки чанк должен быть инициализирован");
    }
    std::vector<uint8_t>& data = data_.GetData();
    data.resize(size_);
    stream.CustomRead(data, offset_);
}


std::vector<uint8_t> ByteChunk::GetCompiledChunk() {
    std::vector<uint8_t> compiled;
    size_ = data_.GetData().size();
    compiled.resize(static_cast<int>(len_num_size_) + size_);
    std::vector<uint8_t> num = GetSizeVector();
    std::memcpy(compiled.data(), num.data(), num.size());
    std::vector<uint8_t>& data = data_.GetData();
    std::memcpy(compiled.data() + num.size(), data.data(), data.size());  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return compiled;
}


bool ByteChunk::IsInitialized() {
    return state_ == ChunkInitState::kFileBacked;
}


uint64_t ByteChunk::GetOffset() {
    return offset_;
}


uint64_t ByteChunk::GetSize() {
    return size_;
}


Storage& ByteChunk::GetStorage() {
    return data_;
}


void ByteChunk::SetSizeNum(NumView num) {
    switch (len_num_size_) {
        case (NumSize::kUint8_t) : {
            size_ = num.GetNum<uint8_t>();
            break;
        }
        case (NumSize::kUint16_t) : {
            size_ = num.GetNum<uint16_t>();
            break;
        }
        case (NumSize::kUint32_t) : {
            size_ = num.GetNum<uint32_t>();
            break;
        }
        case (NumSize::kUint64_t) : {
            size_ = num.GetNum<uint64_t>();
            break;
        }
    }
}


std::vector<uint8_t> ByteChunk::GetSizeVector() {
    std::vector<uint8_t> num;
    NumView view(num, num_endianess_);
    switch (len_num_size_) {
        case (NumSize::kUint8_t) : {
            view.SetNum<uint8_t>(size_);
            break;
        }
        case (NumSize::kUint16_t) : {
            view.SetNum<uint16_t>(size_);
            break;
        }
        case (NumSize::kUint32_t) : {
            view.SetNum<uint32_t>(size_);
            break;
        }
        case (NumSize::kUint64_t) : {
            view.SetNum<uint64_t>(size_);
            break;
        }
    }
    return num;
}

} // namespace wiseio
