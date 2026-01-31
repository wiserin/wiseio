#include <cstddef>  // Copyright 2025 wiserin
#include <stdexcept>
#include <string>
#include <utility>
#include <errno.h>

#include <core.h>

#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"
#include "logging/logger.hpp"


namespace wiseio {

class segmentation_fault : public std::exception {
public:
    const char* what() const noexcept override {
        return "Segmentation fault (core dumped)";
    }
};


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
    size_t fsize = GetFileSize();
    if (position > fsize) {
        throw segmentation_fault();
    }
    cursor_ = position;
}


size_t Stream::GetCursor() {
    return cursor_;
}


bool Stream::IsEOF() const {
    return is_eof_;
}


void Stream::FdCheck() const {
    if (fd_ == -1) {
        logger_.Critical("Обращение к файлу через закрытый fd");
        throw std::runtime_error("fd error");
    }
}


void Stream::Close() {
    wcore_close(fd_);
}


Stream::~Stream() {
    wcore_close(fd_);
}


bool Stream::Open(const char* path) {
    fd_ = -1;
    switch (mode_) {
        case (OpenMode::kRead) : {
            fd_ = wcore_o_read(path);
            break;
        }
        case (OpenMode::kWrite) : {
            fd_ = wcore_o_write(path);
            break;
        }
        case (OpenMode::kAppend) : {
            fd_ = wcore_o_append(path);
            break;
        }
        case (OpenMode::kReadAndWrite) : {
            fd_ = wcore_read_and_write(path);
            break;
        }
    }

    if (fd_ > 0) {
        logger_.Debug("Файл открыт в режиме " + std::to_string(
            static_cast<int>(mode_)));
        return true;
    } else {
        logger_.Error("Ошибка при открытии файла. FD: " + std::to_string(
            fd_) + " Errno: " + std::to_string(errno));
        return false;
    }
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

