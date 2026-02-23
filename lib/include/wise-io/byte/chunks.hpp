#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "wise-io/schemas.hpp"
#include "wise-io/byte/storage.hpp"
#include "wise-io/byte/views.hpp"
#include "wise-io/stream.hpp"

using str = std::string;


namespace wiseio {

class BaseChunk {  // NOLINT
 public:
    virtual void Init(wiseio::Stream& stream) = 0;
    virtual void Load(wiseio::Stream& stream) = 0;
    [[nodiscard]] virtual std::vector<uint8_t> GetCompiledChunk() = 0;
    [[nodiscard]] virtual bool IsInitialized() = 0;

    [[nodiscard]] virtual uint64_t GetOffset() = 0;
    [[nodiscard]] virtual uint64_t GetSize() = 0;
    [[nodiscard]] virtual Storage& GetStorage() = 0;

    virtual ~BaseChunk() = default;
};


class NumChunk : public BaseChunk {
    ChunkInitState state_ = ChunkInitState::kUninitialized;
    Storage data_;
    NumSize size_;
    uint64_t offset_ = 0;

 public:
    NumChunk(NumSize size);

    NumChunk(const NumChunk& another) = delete;
    NumChunk& operator=(const NumChunk& another) = delete;
    NumChunk(NumChunk&& another) noexcept = default;
    NumChunk& operator=(NumChunk&& another) noexcept = default;

    void Init(Stream& stream) override;
    void Load(Stream& stream) override;
    [[nodiscard]] std::vector<uint8_t> GetCompiledChunk() override;
    [[nodiscard]] bool IsInitialized() override;

    [[nodiscard]] uint64_t GetOffset() override;
    [[nodiscard]] uint64_t GetSize() override;
    [[nodiscard]] Storage& GetStorage() override;

    ~NumChunk() override = default;
};


class ByteChunk : public BaseChunk {
    ChunkInitState state_ = ChunkInitState::kUninitialized;
    Endianess num_endianess_;
    Storage data_;
    NumSize len_num_size_;
    uint64_t size_ = 0;
    uint64_t offset_ = 0;

    void SetSizeNum(NumView num);
    [[nodiscard]] std::vector<uint8_t> GetSizeVector();

 public:
    ByteChunk(NumSize size, Endianess num_endianess);

    ByteChunk(const ByteChunk& another) = delete;
    ByteChunk& operator=(const ByteChunk& another) = delete;
    ByteChunk(ByteChunk&& another) noexcept = default;
    ByteChunk& operator=(ByteChunk&& another) noexcept = default;

    void Init(Stream& stream) override;
    void Load(Stream& stream) override;
    [[nodiscard]] std::vector<uint8_t> GetCompiledChunk() override;
    [[nodiscard]] bool IsInitialized() override;

    [[nodiscard]] uint64_t GetOffset() override;
    [[nodiscard]] uint64_t GetSize() override;
    [[nodiscard]] Storage& GetStorage() override;

    ~ByteChunk() override = default;
};


class ValidateChunk : public BaseChunk {
    ChunkInitState state_ = ChunkInitState::kUninitialized;
    std::vector<uint8_t> target_value_;
    Storage data_;
    uint64_t size_ = 0;
    uint64_t offset_ = 0;

    [[nodiscard]] bool Validate(const std::vector<uint8_t>& data) const;

 public:
    ValidateChunk(size_t size, std::vector<uint8_t>&& target_value);

    ValidateChunk(const ValidateChunk& another) = delete;
    ValidateChunk& operator=(const ValidateChunk& another) = delete;
    ValidateChunk(ValidateChunk&& another) noexcept = default;
    ValidateChunk& operator=(ValidateChunk&& another) noexcept = default;

    void Init(Stream& stream) override;
    void Load(Stream& stream) override;
    [[nodiscard]] std::vector<uint8_t> GetCompiledChunk() override;
    [[nodiscard]] bool IsInitialized() override;

    [[nodiscard]] uint64_t GetOffset() override;
    [[nodiscard]] uint64_t GetSize() override;
    [[nodiscard]] Storage& GetStorage() override;

    ~ValidateChunk() override = default;
};


[[nodiscard]] std::unique_ptr<BaseChunk> MakeNumChunk(NumSize size);
[[nodiscard]] std::unique_ptr<BaseChunk> MakeByteChunk(NumSize len_num_size, Endianess num_endianess = Endianess::kLittleEndian);
[[nodiscard]] std::unique_ptr<BaseChunk> MakeValidateChunk(uint64_t size, std::vector<uint8_t>&& target_value);

} // namespace wiseio
