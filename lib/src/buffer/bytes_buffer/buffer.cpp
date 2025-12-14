#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "wise-io/buffer.hpp"


namespace wiseio {

uint8_t* BytesIOBuffer::GetDataPtr() {
    return data_.data();
}


const uint8_t* BytesIOBuffer::GetDataPtr() const {
    return data_.data();
}


size_t BytesIOBuffer::GetBufferSize() const {
    return data_.size();
}


void BytesIOBuffer::ResizeBuffer(size_t size) {
    data_.resize(size);
}


void BytesIOBuffer::SetCursor(size_t position) {
    if (position > data_.size()) {
        throw std::out_of_range(
            "Индекс должен находиться в пределах размера буфера. Запрошенная длинна: "
            + std::to_string(position) + " реальный размер буфера: "
            + std::to_string(data_.size()));
    }

    cursor_ = position;
}


bool BytesIOBuffer::IsData() const {
    if (cursor_ < data_.size()) {
        return true;
    }
    return false;
}


std::vector<uint8_t> BytesIOBuffer::ReadFromBuffer(size_t size) {
    std::vector<uint8_t> buffer;
    buffer.reserve(128);

    while (size > 0 && cursor_ < data_.size()) {
        buffer.push_back(data_[cursor_]);
        ++cursor_;
        --size;
    }

    return buffer;
}


void BytesIOBuffer::AddDataToBuffer(const std::vector<uint8_t>& data) {
    data_.insert(data_.end(), data.begin(), data.end());
}


void BytesIOBuffer::Clear() {
    data_.resize(0);
    data_.shrink_to_fit();
    data_.reserve(128);
    cursor_ = 0;
}



} // namespase wiseio