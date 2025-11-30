#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <unistd.h>
#include <utility>

#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"
#include "logging/logger.hpp"


namespace wiseio {

Stream::Stream(
        OpenMode io_mode,
        uint64_t buffer_size) 
        : buffer_size_(buffer_size)
        , mode_(io_mode) {}


Stream::Stream(Stream&& another)
        : fd_(another.fd_)
        , buffer_size_(another.buffer_size_)
        , is_eof_(another.is_eof_)
        , mode_(another.mode_)
        , cursor_(another.cursor_)
        , logger_(std::move(another.logger_)) {

    another.fd_ = -1;
}


Stream& Stream::operator=(Stream&& another) {
    if (fd_ != -1) {
        Close();
    }

    fd_ = another.fd_;
    buffer_size_ = another.buffer_size_;
    is_eof_ = another.is_eof_;
    mode_ = another.mode_;
    cursor_ = another.cursor_;
    logger_ = std::move(another.logger_);

    another.fd_ = -1;
    
    return *this;
}


void Stream::Close() {
    close(fd_);
}


Stream::~Stream() {
    Close();
}


Stream CreateStream(const char* name, OpenMode mode, uint64_t buffer_size) {
    Stream stream {mode, buffer_size};

    stream.logger_ = logging::Logger {name};
    bool state = stream.Open(name);

    if (!state) {
        throw std::runtime_error("Ошибка при открытии файла");
    }
    return stream;
}


} // namespace wiseio

