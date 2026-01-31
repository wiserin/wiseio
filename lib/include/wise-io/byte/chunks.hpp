#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "wise-io/schemas.hpp"
#include "wise-io/stream.hpp"
#include "wise-io/byte/storage.hpp"

using str = std::string;


namespace wiseio {

class BaseChunk {
 public:
    virtual void Init(wiseio::Stream& stream) = 0;
    virtual void Load(wiseio::Stream& stream) = 0;
    virtual std::vector<uint8_t> GetCompiledChunk() = 0;
    virtual bool IsInitialized() = 0;

    virtual size_t GetOffset() = 0;
    virtual size_t GetSize() = 0;
    virtual Storage& GetStorage() = 0;
    virtual ~BaseChunk() = default;
};


class NumChunk : public BaseChunk {
    ChunkInitState state_ = ChunkInitState::kUninitialized;
    Storage data_;
    NumSize size_;
    size_t offset_ = 0;

 public:
    NumChunk(NumSize size);
    void Init(Stream& stream) override;
    void Load(Stream& stream) override;
    std::vector<uint8_t> GetCompiledChunk() override;
    bool IsInitialized() override;

    size_t GetOffset() override;
    size_t GetSize() override;
    Storage& GetStorage() override;
};


class ByteChunk : public BaseChunk {
    ChunkInitState state_ = ChunkInitState::kUninitialized;
    Storage data_;
    NumSize len_num_size_;
    size_t size_ = 0;
    size_t offset_ = 0;

 public:
    ByteChunk(NumSize size);
    void Init(Stream& stream) override;
    void Load(Stream& stream) override;
    std::vector<uint8_t> GetCompiledChunk() override;
    bool IsInitialized() override;

    size_t GetOffset() override;
    size_t GetSize() override;
    Storage& GetStorage() override;
};


class ValidateChunk : public BaseChunk {
    ChunkInitState state_ = ChunkInitState::kUninitialized;
    Storage data;
    size_t size_ = 0;
    size_t offset_ = 0;

 public:
    ValidateChunk(size_t size);
    void Init(Stream& stream) override;
    void Load(Stream& stream) override;
    std::vector<uint8_t> GetCompiledChunk() override;
    bool IsInitialized() override;

    size_t GetOffset() override;
    size_t GetSize() override;
    Storage& GetStorage() override;

};


std::unique_ptr<BaseChunk> MakeNumChunk(NumSize size);
std::unique_ptr<BaseChunk> MakeByteChunk(NumSize len_num_size);
std::unique_ptr<BaseChunk> MakeValidateChunk(size_t size);

} // namespace wiseio
