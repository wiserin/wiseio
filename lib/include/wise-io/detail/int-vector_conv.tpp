#pragma once  // Copyright 2025 wiserin
#include <bit>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>


#include "wise-io/schemas.hpp"
#include "wise-io/concepts.hpp"


using str = std::string;


namespace wiseio {

template<Integral T>
T FromVector(const std::vector<uint8_t>& data, wiseio::Endianess source_endian) {
    if (sizeof(T) != data.size()) {
        throw std::logic_error("Размеры не совпадают");
    }

    T num;
    std::memcpy(&num, data.data(), sizeof(T));

    if ((std::endian::native == std::endian::little && source_endian == Endianess::kBigEndian) ||
        (std::endian::native == std::endian::big    && source_endian == Endianess::kLittleEndian)) {
        num = std::byteswap<T>(num);
    }
    return num;
}


template<Integral T>
std::vector<uint8_t> ToVector(T num, wiseio::Endianess target_endian) {
    std::vector<uint8_t> data;
    data.resize(sizeof(T));

    if ((std::endian::native == std::endian::little && target_endian == Endianess::kBigEndian) ||
        (std::endian::native == std::endian::big    && target_endian == Endianess::kLittleEndian)) {
        num = std::byteswap<T>(num);
    }
    std::memcpy(data.data(), &num, sizeof(T));
    return data;
}

} // namespace wiseio
