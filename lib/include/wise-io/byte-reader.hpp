#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

#include <wise-io/schemas.hpp>
#include <wise-io/stream.hpp>


using str = std::string;


namespace wiseio {


class Storage {
    std::vector<uint8_t> data_;
    StorageState state_ = StorageState::kClean;
    str file_name_;

 public:
    void Commit();
    void LoadData(const wiseio::Stream& stream);
};


class ByteNum {
    std::vector<uint8_t> data_;

 public:
    void SetNum(const std::vector<uint8_t> data);

    template<typename T>
    T GetNum();

};


class BaseChunk {
 public:
    virtual void Load(wiseio::Stream& stream) = 0;
    virtual Storage& GetStorage() = 0;
    virtual ~BaseChunk() = default;
};


class NumChunk : public BaseChunk {
    Storage data_;
    NumSize size_;
    size_t offset_;

 public:
    void Load(wiseio::Stream& stream) override;

    Storage& GetStorage() override;

};


class ByteChunk : public BaseChunk {
    Storage data_;
    NumSize len_num_size_;
    size_t size_ = 0;
    size_t offset_ = 0;

 public:
    ByteChunk(NumSize size);
    void Load(wiseio::Stream& stream) override;

    Storage& GetStorage() override;
};


class ValidateChunk : public BaseChunk {
    Storage data;
    size_t size_;
    size_t offset_;

 public:
    void Load(wiseio::Stream& stream) override;

    Storage& GetStorage() override;

};


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
    std::unordered_map<str, BaseChunk&> index_;
    ByteFileEngine file_engine_;

    bool IsNameInIndex(const str& name);

public:
    ByteFile(const char* file_name);

    void AddChunk(std::unique_ptr<BaseChunk> chunk, str&& name);
    BaseChunk& GetChunk(const str& name);

    void ReadChunksFromFile();

};


std::unique_ptr<BaseChunk> MakeNumChunk(NumSize size);
std::unique_ptr<BaseChunk> MakeByteChunk(NumSize len_num_size);
std::unique_ptr<BaseChunk> MakeByteChunk(NumSize len_num_size);

} // namespace wiseio
