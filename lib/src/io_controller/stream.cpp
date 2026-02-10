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


Stream::Stream(
        OpenMode io_mode,
        const char* file_name)
    : mode_(io_mode)
    , file_path_(file_name) {}


Stream::Stream(Stream&& another)
        : fd_(another.fd_)
        , is_eof_(another.is_eof_)
        , mode_(another.mode_)
        , cursor_(another.cursor_)
        , file_path_(std::move(another.file_path_))
        , logger_(std::move(another.logger_)) {

    another.fd_ = -1;
}


Stream& Stream::operator=(Stream&& another) {
    if (IsOpen() && fd_ != another.fd_) {
        Close();
    }

    fd_ = another.fd_;
    is_eof_ = another.is_eof_;
    mode_ = another.mode_;
    cursor_ = another.cursor_;
    file_path_ = another.file_path_;
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


void Stream::SetDelete() const {
    if (IsOpen()) {
        wcore_unlink_file(file_path_.c_str());
    }
}


size_t Stream::GetCursor() {
    return cursor_;
}


bool Stream::IsEOF() const {
    return is_eof_;
}


bool Stream::IsOpen() const {
    return fd_ >= 0;
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


bool Stream::Open() {
    fd_ = -1;
    switch (mode_) {
        case (OpenMode::kRead) : {
            fd_ = wcore_o_read(file_path_.c_str());
            break;
        }
        case (OpenMode::kWrite) : {
            fd_ = wcore_o_write(file_path_.c_str());
            break;
        }
        case (OpenMode::kAppend) : {
            fd_ = wcore_o_append(file_path_.c_str());
            break;
        }
        case (OpenMode::kReadAndWrite) : {
            fd_ = wcore_read_and_write(file_path_.c_str());
            break;
        }
    }

    if (fd_ >= 0) {
        logger_.Debug("Файл открыт в режиме " + std::to_string(
            static_cast<int>(mode_)));
        return true;
    } else {
        logger_.Error("Ошибка при открытии файла. FD: " + std::to_string(
            fd_) + " Errno: " + std::to_string(errno));
        return false;
    }
}


Stream CreateStream(const char* name, OpenMode mode, bool is_temp) {
    Stream stream (mode, name);

    stream.logger_ = logging::Logger {name};
    bool state = stream.Open();

    if (!state) {
        throw std::runtime_error("Ошибка при открытии файла");
    }
    if (is_temp) {
        stream.SetDelete();
    }
    return stream;
}


Stream CreateStream(const std::filesystem::path& name, OpenMode mode, bool is_temp) {
    Stream stream {mode, name.c_str()};

    stream.logger_ = logging::Logger {name};
    bool state = stream.Open();

    if (!state) {
        throw std::runtime_error("Ошибка при открытии файла");
    }
    if (is_temp) {
        stream.SetDelete();
    }
    return stream;
}


} // namespace wiseio

