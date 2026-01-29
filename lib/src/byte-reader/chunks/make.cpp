#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <memory>
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

std::unique_ptr<BaseChunk> MakeByteChunk(NumSize len_num_size) {
    std::unique_ptr<BaseChunk> chunk = std::make_unique<ByteChunk>(len_num_size);
    return chunk;
}


} // namespace wiseio
