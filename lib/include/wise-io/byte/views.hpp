#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <wise-io/schemas.hpp>
#include <wise-io/stream.hpp>
#include <wise-io/utils.hpp>


using str = std::string;


namespace wiseio {

class NumView {  // TODO Переписать на weak
    std::vector<uint8_t>& data_;  // NOLINT
    Endianness endianess_;

 public:
    NumView(std::vector<uint8_t>& data, Endianness endianess = Endianness::kLittleEndian);

    template<typename T>
    [[nodiscard]] T GetNum();

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
