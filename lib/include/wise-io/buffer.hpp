#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <wise-io/schemas.hpp>


using str = std::string;

namespace wiseio {

class IOBuffer {  // NOLINT
 public:
    virtual void ResizeBuffer(size_t size) = 0;
    [[nodiscard]] virtual size_t GetBufferSize() const = 0;
    [[nodiscard]] virtual uint8_t* GetDataPtr() = 0;
    [[nodiscard]] virtual const uint8_t* GetDataPtr() const = 0;
    virtual ~IOBuffer() = default;
};


class BytesIOBuffer : public IOBuffer {
    std::vector<uint8_t> data_;
    size_t cursor_ = 0;

 public:
    BytesIOBuffer() = default;
    BytesIOBuffer(const BytesIOBuffer& another) = default;
    BytesIOBuffer& operator=(const BytesIOBuffer& another) = default;
    BytesIOBuffer(BytesIOBuffer&& another) noexcept = default;
    BytesIOBuffer& operator=(BytesIOBuffer&& another) = default;
    
    [[nodiscard]] uint8_t* GetDataPtr() override;
    [[nodiscard]] const uint8_t* GetDataPtr() const override;
    [[nodiscard]] size_t GetBufferSize() const override;
    void ResizeBuffer(size_t size) override;

    void SetCursor(size_t position);
    void AddDataToBuffer(const std::vector<uint8_t>& data);

    [[nodiscard]] bool IsData() const;

    [[nodiscard]] std::vector<uint8_t> ReadFromBuffer(size_t size);
    void Clear();

    ~BytesIOBuffer() override = default;
};


class StringIOBuffer : public IOBuffer {
    std::vector<char> data_;
    size_t cursor_ = 0;
    Encoding encoding_ = Encoding::kUTF_8;

    bool ignore_comments_ = false;
    bool ignore_blank_ = false;

    [[nodiscard]] bool Validate(std::vector<char>& line) const;

    [[nodiscard]] bool IsBlank(const std::vector<char>& line) const;
    [[nodiscard]] bool CommentChecker(std::vector<char>& line) const;

    void DeleteComment(std::vector<char>& line) const;

    [[nodiscard]] std::vector<char> ReadLine();

 public:
    StringIOBuffer() = default;
    StringIOBuffer(const StringIOBuffer& another) = default;
    StringIOBuffer& operator=(const StringIOBuffer& another) = default;
    StringIOBuffer(StringIOBuffer&& another) noexcept = default;
    StringIOBuffer& operator=(StringIOBuffer&& another) = default;

    [[nodiscard]] uint8_t* GetDataPtr() override;
    [[nodiscard]] const uint8_t* GetDataPtr() const override;
    [[nodiscard]] size_t GetBufferSize() const override;
    void ResizeBuffer(size_t size) override;

    void SetCursor(size_t position);
    void SetIgnoreBlank(bool state);
    void SetIgnoreComments(bool state);
    void SetEncoding(Encoding encoding);

    void AddDataToBuffer(const str& data);

    [[nodiscard]] str GetLine();
    [[nodiscard]] size_t GetLen() const;
    [[nodiscard]] bool IsLines() const;

    [[nodiscard]] str ReadFromBuffer(size_t size);
    void Clear();

    ~StringIOBuffer() override = default;
};


} // namespace wiseio
