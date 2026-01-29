#pragma once  // Copyright 2025 wiserin
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <string>

#include <wise-io/schemas.hpp>
#include <wise-io/stream.hpp>


using str = std::string;


template<typename T>
T FromVectorLittleEndian(const std::vector<uint8_t>& data) {
    if (sizeof(T) != data.size()) {
        throw std::logic_error("Размер типа переменной не может быть меньше длинны самого массива");
    }
    T num;
}


template<typename T>
T FromVectorBigEndian(const std::vector<uint8_t>& data) {
    if (sizeof(T) != data.size()) {
        throw std::logic_error("Размер типа переменной не может быть меньше длинны самого массива");
    }
    T num;

}


template<typename T>
std::vector<uint8_t> ToVectorLittleEndian(T num) {

}


template<typename T>
std::vector<uint8_t> ToVectorBigEndian(T num) {

}


namespace wiseio {

class ByteNum {
    std::vector<uint8_t> data_;
 public:
    void SetNum(const std::vector<uint8_t> data);

    template<typename T>
    T GetNum(Endiannes endian);
};


template<typename T>
T ByteNum::GetNum(Endiannes endian) {
    if (endian == Endiannes::kLittleEndian) {

    } else {

    }
}

} // namespace wiseio
