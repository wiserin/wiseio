#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <string>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/buffer.hpp"


using str = std::string;

namespace wiseio {

ValidateChunk::ValidateChunk(size_t size)
        : size_(size) {}


void ValidateChunk::Init(wiseio::Stream& stream) {

}


void ValidateChunk::Load(Stream& stream) {

}


std::vector<uint8_t> ValidateChunk::GetCompiledChunk() {
    
}


bool ValidateChunk::IsInitialized() {

}


uint64_t ValidateChunk::GetOffset() {

}


uint64_t ValidateChunk::GetSize() {

}


Storage& ValidateChunk::GetStorage() {

}

} // namespace wiseio
