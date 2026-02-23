#include <cstdint>  // Copyright 2025 wiserin
#include <vector>

#include "wise-io/byte/views.hpp"


using str = std::string;

namespace wiseio {


NumView::NumView(std::vector<uint8_t>& data, Endianness endianess)
        : data_(data)
        , endianess_(endianess) {}

} // namespace wiseio
