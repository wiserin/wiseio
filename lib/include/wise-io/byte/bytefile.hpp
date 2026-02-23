#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/concepts.hpp"


using str = std::string;


namespace wiseio {

class ByteFileEngine {
    wiseio::Stream istream_;
    std::filesystem::path file_name_;

 public:
    ByteFileEngine() = default;
    ByteFileEngine(const ByteFileEngine& another) = delete;
    ByteFileEngine& operator=(const ByteFileEngine& another) = delete;
    ByteFileEngine(ByteFileEngine&& another) = default;
    ByteFileEngine& operator=(ByteFileEngine&& another) = default;

    ByteFileEngine(const char* file_name);
    void InitChunks(const std::vector<std::unique_ptr<BaseChunk>>& chunks);
    void ReadChunk(BaseChunk& chunk);
    void CompileFile(const std::vector<std::unique_ptr<BaseChunk>>& chunks);

    ~ByteFileEngine() = default;
};


template <Hashable T = str>
class ByteFile {
    std::vector<std::unique_ptr<BaseChunk>> layout_;
    std::unordered_map<T, BaseChunk*> index_;
    ByteFileEngine file_engine_;

    bool IsNameInIndex(const T& name);

public:
    ByteFile() = default;
    ByteFile(const ByteFile<T>& another) = delete;
    ByteFile<T>& operator=(const ByteFile<T> another) = delete;
    ByteFile(ByteFile<T>&& another) = default;
    ByteFile<T>& operator=(ByteFile<T>&& another) = default;

    ByteFile(const char* file_name);

    void AddChunk(std::unique_ptr<BaseChunk> chunk, T&& name);
    void AddChunk(std::unique_ptr<BaseChunk> chunk, const T& name);
    BaseChunk& GetChunk(const T& name);
    BaseChunk& GetAndLoadChunk(const T& name);

    void InitChunksFromFile();
    void Compile();

    ~ByteFile() = default;
};


} // namespace wiseio

#include "wise-io/byte/detail/bytefile.tpp"
