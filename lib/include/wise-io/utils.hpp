#pragma once  // Copyright 2025 wiserin
#include <cstdint>
#include <vector>
#include <string>

#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/concepts.hpp"


using str = std::string;

namespace wiseio {

template<Integral T>
T FromVector(const std::vector<uint8_t>& data, wiseio::Endianess source_endian);


template<Integral T>
std::vector<uint8_t> ToVector(T num, wiseio::Endianess target_endian);


class FileNamer {
    inline static uint64_t current = 0;

 public:
    static str GetName();
};

} // namespace wiseio


#include "wise-io/detail/int-vector_conv.tpp"
