#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

#include <wise-io/schemas.hpp>
#include <wise-io/stream.hpp>
#include <wise-io/utils.hpp>


using str = std::string;


namespace wiseio {

class NumView {
    std::vector<uint8_t>& data_;
    Endianess endianess_;

 public:
    NumView(std::vector<uint8_t>& data, Endianess endianess = Endianess::kLittleEndian);

    template<typename T>
    T GetNum();

    template<typename T>
    void SetNum(T num);
};


template <typename T>
T NumView::GetNum() {
    return FromVector<T>(data_, endianess_);
}


template <typename T>
void NumView::SetNum(T num) {
    data_ = ToVector<T>(num, endianess_);
}

} // namespace wiseio
