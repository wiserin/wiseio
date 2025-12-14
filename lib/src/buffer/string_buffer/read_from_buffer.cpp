#include <cstddef>  // Copyright 2025 wiserin
#include <vector>

#include "wise-io/buffer.hpp"


namespace wiseio {


std::vector<char> StringIOBuffer::ReadLine() {
    if (cursor_ >= data_.size()) {
        return std::vector<char>();
    }
    std::vector<char> buffer;
    buffer.reserve(128);

    while (cursor_ < data_.size() && data_[cursor_] != '\n') {
        buffer.push_back(data_[cursor_]);
        ++cursor_;
    }

    if (cursor_ < data_.size() && data_[cursor_] == '\n') {
        ++cursor_;
    }

    return buffer;
}


} // namespase wiseio