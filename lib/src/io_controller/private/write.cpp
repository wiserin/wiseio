#include <cstddef>  // Copyright 2025 wiserin
#include <string>
#include <unistd.h>
#include <fcntl.h>

#include "wise-io/stream.hpp"


namespace wiseio {

bool Stream::AWrite(const uint8_t* buffer, size_t buffer_size) {
    size_t written = 0;

    while (written < buffer_size) {
        ssize_t res = write(fd_, buffer + written, buffer_size - written);

        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            logger_.Error("Ошибка записи в файл. Errno: " + std::to_string(errno));
            return false;
        }
        written += res;
    }

    return true;
}


bool Stream::CWrite(const uint8_t* buffer, size_t buffer_size) {
    size_t written = 0;

    while (written < buffer_size) {
        ssize_t res = pwrite(fd_, buffer + written, buffer_size - written, cursor_ + written);

        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            logger_.Error("Ошибка записи в файл. Errno: " + std::to_string(errno));
            return false;
        }
        written += res;
    }

    cursor_ += written;

    return true;
}


bool Stream::CustomWrite(const uint8_t* buffer, size_t offset, size_t buffer_size) {
    size_t written = 0;

    while (written < buffer_size) {
        ssize_t res = pwrite(fd_, buffer + written, buffer_size - written, offset + written);

        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            logger_.Error("Ошибка записи в файл. Errno: " + std::to_string(errno));
            return false;
        }
        written += res;
    }

    return true;
}

} // namespace wiseio
