#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <string>
#include <unordered_map>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/stream.hpp"


using str = std::string;


namespace wiseio {


class ByteFileEngine {
    wiseio::Stream istream_;
    const char* file_name_;

 public:
    ByteFileEngine(const char* file_name);
    void InitChunks(const std::vector<std::unique_ptr<BaseChunk>>& chunks);
    void ReadChunk(BaseChunk& chunk);
    void CompileFile(const std::vector<std::unique_ptr<BaseChunk>>& chunks);
};

class ByteFile {
    std::vector<std::unique_ptr<BaseChunk>> layout_;
    std::unordered_map<str, BaseChunk*> index_;
    ByteFileEngine file_engine_;

    bool IsNameInIndex(const str& name);

public:
    ByteFile(const char* file_name);

    void AddChunk(std::unique_ptr<BaseChunk> chunk, str&& name);
    BaseChunk& GetChunk(const str& name);
    BaseChunk& GetAndLoadChunk(const str& name);

    void InitChunksFromFile();
    void Compile();

};

} // namespace wiseio
