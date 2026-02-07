#include <cstddef>  // Copyright 2025 wiserin
#include <memory>
#include <sys/types.h>
#include <string>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/schemas.hpp"


using str = std::string;

namespace wiseio {

std::unique_ptr<BaseChunk> MakeByteChunk(NumSize len_num_size, Endianess num_endianess) {
    std::unique_ptr<BaseChunk> chunk = std::make_unique<ByteChunk>(
        len_num_size, num_endianess);
    return chunk;
}


std::unique_ptr<BaseChunk> MakeNumChunk(NumSize size) {
    std::unique_ptr<BaseChunk> chunk = std::make_unique<NumChunk>(size);
    return chunk;
}


std::unique_ptr<BaseChunk> MakeValidateChunk(uint64_t size) {
    std::unique_ptr<BaseChunk> chunk = std::make_unique<ValidateChunk>(size);
    return chunk;
}


} // namespace wiseio
