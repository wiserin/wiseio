#pragma once  // Copyright 2025 wiserin
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

#include <wise-io/schemas.hpp>


using str = std::string;

namespace wiseio {

class IOBuffer {
 public:
    virtual void ResizeBuffer(size_t size) = 0;
    virtual size_t GetBufferSize() const = 0;
    virtual uint8_t* GetDataPtr() = 0;
    virtual const uint8_t* GetDataPtr() const = 0;
    virtual ~IOBuffer() = default;
};


class BytesIOBuffer : public IOBuffer {
    std::vector<uint8_t> data_;
    size_t cursor_ = 0;

 public:
    uint8_t* GetDataPtr() override;
    const uint8_t* GetDataPtr() const override;
    size_t GetBufferSize() const override;
    void ResizeBuffer(size_t size) override;

    void SetCursor(size_t position);
    void AddDataToBuffer(const std::vector<uint8_t>& data);

    bool IsData() const;

    std::vector<uint8_t> ReadFromBuffer(size_t size);
    void Clear();
};


class StringIOBuffer : public IOBuffer {
    std::vector<char> data_;
    size_t cursor_ = 0;
    Encoding encoding_ = Encoding::kUTF_8;

    bool ignore_comments_ = false;
    bool ignore_blank_ = false;

    bool Validate(std::vector<char>& line) const;

    bool IsBlank(const std::vector<char>& line) const;
    bool CommentChecker(std::vector<char>& line) const;

    void DeleteComment(std::vector<char>& line) const;

    std::vector<char> ReadLine();

 public:
    uint8_t* GetDataPtr() override;
    const uint8_t* GetDataPtr() const override;
    size_t GetBufferSize() const override;
    void ResizeBuffer(size_t size) override;

    void SetCursor(size_t position);
    void SetIgnoreBlank(bool state);
    void SetIgnoreComments(bool state);
    void SetEncoding(Encoding encoding);

    void AddDataToBuffer(const str& data);

    str GetLine();
    size_t GetLen() const;
    bool IsLines() const;

    str ReadFromBuffer(size_t size);
    void Clear();
};


} // namespace wiseio
