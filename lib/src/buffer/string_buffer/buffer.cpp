#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "wise-io/buffer.hpp"
#include "wise-io/schemas.hpp"


namespace wiseio {

uint8_t* StringIOBuffer::GetDataPtr() {
    return reinterpret_cast<uint8_t*>(data_.data());
}


const uint8_t* StringIOBuffer::GetDataPtr() const {
    return reinterpret_cast<const uint8_t*>(data_.data());
}


size_t StringIOBuffer::GetBufferSize() const {
    return data_.size();
}


void StringIOBuffer::ResizeBuffer(size_t size) {
    data_.resize(size);
}


void StringIOBuffer::SetCursor(size_t position) {
    if (position > data_.size()) {
        throw std::out_of_range(
            "Индекс должен находиться в пределах размера буфера. Запрошенная длинна: "
            + std::to_string(position) + " реальный размер буфера: "
            + std::to_string(data_.size()));
    }

    cursor_ = position;
}


void StringIOBuffer::SetIgnoreComments(bool state) {
    ignore_comments_ = state;
}


void StringIOBuffer::SetEncoding(Encoding encoding) {
    encoding_ = encoding;
}


size_t StringIOBuffer::GetLen() const {
    return data_.size() / static_cast<int>(encoding_);
}


bool StringIOBuffer::IsLines() const {
    if (cursor_ < data_.size()) {
        return true;
    }
    return false;
}


str StringIOBuffer::GetLine() {
    std::vector<char> buffer = ReadLine();

    while (!Validate(buffer) && IsLines()) {
        buffer = ReadLine();
    }

    return str(buffer.data(), buffer.size());
}


void StringIOBuffer::AddDataToBuffer(const str& data) {
    data_.insert(data_.end(), data.begin(), data.end());
}


void StringIOBuffer::Clear() {
    data_.resize(0);
    data_.shrink_to_fit();
    data_.reserve(128);
    cursor_ = 0;
}



} // namespase wiseio