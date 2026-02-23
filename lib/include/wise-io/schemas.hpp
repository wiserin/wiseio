#pragma once  // Copyright 2025 wiserin
#include <cstdint>

namespace wiseio {

enum class OpenMode : uint8_t {
    kRead = 0,
    kWrite,
    kAppend,
    kReadAndWrite,
    kDefault
};


enum class Encoding : uint8_t {
    kUTF_8 = 1,
    kUTF_16
};


enum class NumSize : uint8_t {
    kUint8_t = 1,
    kUint16_t = 2,
    kUint32_t = 4,
    kUint64_t = 8,
};


enum class StorageState : uint8_t {
    kClean = 0,
    kDirty,
    kCommited
};

enum class ChunkInitState : uint8_t {
    kUninitialized,
    kFileBacked
};


enum class Endianness : uint8_t {
    kLittleEndian = 0,
    kBigEndian
};


} // namespace wiseio
