#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
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


ByteFileEngine::ByteFileEngine(const char* file_name)
        : file_name_(file_name)
        , istream_(wiseio::CreateStream(
            file_name, OpenMode::kRead)) {}


void ByteFileEngine::InitChunks(const std::vector<std::unique_ptr<BaseChunk>>& chunks) {
    for (int i = 0; i < chunks.size(); ++i) {
        BaseChunk& chunk = *chunks[i];
        chunk.load(istream_);
    }
}


} // namespace wiseio
