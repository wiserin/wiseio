#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/byte/bytefile.hpp"
#include "wise-io/byte/storage.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/utils.hpp"


using str = std::string;

namespace wiseio {


ByteFileEngine::ByteFileEngine(const char* file_name)
        : file_name_(file_name)
        , istream_(wiseio::CreateStream(
            file_name, OpenMode::kReadAndWrite)) {}


void ByteFileEngine::InitChunks(const std::vector<std::unique_ptr<BaseChunk>>& chunks) {
    for (const std::unique_ptr<BaseChunk>& chunk : chunks) {
        chunk->Init(istream_);
    }
}


void ByteFileEngine::ReadChunk(BaseChunk& chunk) {
    chunk.Load(istream_);
}


void ByteFileEngine::CompileFile(const std::vector<std::unique_ptr<BaseChunk>>& chunks) {
    Stream ostream = CreateStream(file_name_.root_path() / FileNamer::GetName(), OpenMode::kAppend);

    for (const std::unique_ptr<BaseChunk>& chunk : chunks) {
        if (chunk->GetStorage().IsChanged()) {
            std::vector<uint8_t> data = chunk->GetCompiledChunk();
            ostream.AWrite(data);
        } else if (chunk->IsInitialized()) {
            ReadChunk(*chunk);
            std::vector<uint8_t> data = chunk->GetCompiledChunk();
            ostream.AWrite(data);
        }
    }
    istream_.SetDelete();
    istream_.Close();
    ostream.Rename(file_name_.filename());
}


} // namespace wiseio
