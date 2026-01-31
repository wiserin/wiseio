#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <string>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"


using str = std::string;

namespace wiseio {


ByteChunk::ByteChunk(NumSize size)
    : len_num_size_(size) {}


void ByteChunk::Init(wiseio::Stream& stream) {

}


void ByteChunk::Load(Stream& stream) {

}


std::vector<uint8_t> ByteChunk::GetCompiledChunk() {
    
}


bool ByteChunk::IsInitialized() {

}


size_t ByteChunk::GetOffset() {

}


size_t ByteChunk::GetSize() {

}


Storage& ByteChunk::GetStorage() {

}


} // namespace wiseio
