#include <cstddef>  // Copyright 2025 wiserin
#include <stdexcept>
#include <unistd.h>
#include <utility>

#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"
#include "logging/logger.hpp"


namespace wiseio {

Stream::Stream(OpenMode io_mode)
        : mode_(io_mode) {}


Stream::Stream(Stream&& another)
        : fd_(another.fd_)
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
    is_eof_ = another.is_eof_;
    mode_ = another.mode_;
    cursor_ = another.cursor_;
    logger_ = std::move(another.logger_);

    another.fd_ = -1;
    
    return *this;
}

void Stream::SetCursor(size_t position) {
    cursor_ = position;
}


bool Stream::IsEOF() {
    return is_eof_;
}


void Stream::Close() {
    close(fd_);
}


Stream::~Stream() {
    Close();
}


Stream CreateStream(const char* name, OpenMode mode) {
    Stream stream {mode};

    stream.logger_ = logging::Logger {name};
    bool state = stream.Open(name);

    if (!state) {
        throw std::runtime_error("Ошибка при открытии файла");
    }
    return stream;
}


} // namespace wiseio

