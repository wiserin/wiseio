#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <string>

#include "wise-io/byte-reader.hpp"
#include "wise-io/core.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"


using str = std::string;

namespace wiseio {


ByteChunk::ByteChunk(NumSize size)
    : len_num_size_(size) {}


void ByteChunk::Load(wiseio::Stream& stream) {
    std::
}

} // namespace wiseio
