#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <string>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/byte/storage.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/byte/bytefile.hpp"


using str = std::string;

namespace wiseio {


ByteFileEngine::ByteFileEngine(const char* file_name)
        : file_name_(file_name)
        , istream_(wiseio::CreateStream(
            file_name, OpenMode::kReadAndWrite)) {}


void ByteFileEngine::InitChunks(const std::vector<std::unique_ptr<BaseChunk>>& chunks) {
    for (int i = 0; i < chunks.size(); ++i) {
        BaseChunk& chunk = *chunks[i];
        chunk.Init(istream_);
    }
}


void ByteFileEngine::ReadChunk(BaseChunk& chunk) {
    chunk.Load(istream_);
}


void ByteFileEngine::CompileFile(const std::vector<std::unique_ptr<BaseChunk>>& chunks) {
    Stream ostream = CreateStream("test_ooo.bin", OpenMode::kAppend);
    for (int i = 0; i < chunks.size(); ++i) {
        BaseChunk& chunk = *chunks[i];

        if (chunk.GetStorage().IsChanged()) {
            std::vector<uint8_t> data = chunk.GetCompiledChunk();
            ostream.AWrite(data);
        } else if (chunk.IsInitialized()) {
            ReadChunk(chunk);
            std::vector<uint8_t> data = chunk.GetCompiledChunk();
            ostream.AWrite(data);
        }
    }
}


} // namespace wiseio
