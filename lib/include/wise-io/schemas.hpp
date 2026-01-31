#pragma once  // Copyright 2025 wiserin


namespace wiseio {

enum class OpenMode {
    kRead = 0,
    kWrite,
    kAppend,
    kReadAndWrite
};


enum class Encoding {
    kUTF_8 = 1,
    kUTF_16
};


enum class NumSize {
    kUint8_t = 1,
    kUint16_t = 2,
    kUint32_t = 4,
    kUint64_t = 8,
};


enum class StorageState {
    kClean = 0,
    kDirty,
    kCommited
};

enum class ChunkInitState {
    kUninitialized,
    kFileBacked
};


enum class Endianess {
    kLittleEndian = 0,
    kBigEndian
};


} // namespace wiseio
