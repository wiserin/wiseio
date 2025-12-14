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


} // namespace wiseio