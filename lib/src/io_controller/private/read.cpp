#include <cstddef>  // Copyright 2025 wiserin
#include <unistd.h>
#include <fcntl.h>

#include "wise-io/stream.hpp"


namespace wiseio {

ssize_t Stream::Read(uint8_t* buffer, size_t offset, size_t buffer_size) {
    size_t count = 0;

    while (count < buffer_size) {
        ssize_t c_bytes = pread(fd_, buffer + count, buffer_size - count, offset + count);

        if (c_bytes + count >= buffer_size) {
            count += c_bytes;
            break;
        } else if (c_bytes == 0) {
            is_eof_ = true;
            logger_.Debug("Достигнут конец файла");
            break;
        } else if (c_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            logger_.Error("Ошибка при чтении файла Errno: " + std::to_string(errno));
            return -1;
        } else {
            count += c_bytes;
        }
    }

    return count;
}


ssize_t Stream::CRead(uint8_t* buffer, size_t buffer_size) {
    size_t count = 0;

    while (count < buffer_size) {
        ssize_t c_bytes = pread(fd_, buffer + count, buffer_size - count, cursor_ + count);

        if (c_bytes + count >= buffer_size) {
            count += c_bytes;
            break;
        } else if (c_bytes == 0) {
            is_eof_ = true;
            logger_.Debug("Достигнут конец файла");
            break;
        } else if (c_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            logger_.Error("Ошибка при чтении файла Errno: " + std::to_string(errno));
            return -1;
        } else {
            count += c_bytes;
        }
    }
    cursor_ += count;

    return count;
}


ssize_t Stream::CustomRead(uint8_t* buffer, size_t offset, size_t buffer_size) {
    size_t count = 0;

    while (count < buffer_size) {
        ssize_t c_bytes = pread(fd_, buffer + count, buffer_size - count, offset + count);

        if (c_bytes + count >= buffer_size) {
            count += c_bytes;
            break;
        } else if (c_bytes == 0) {
            is_eof_ = true;
            logger_.Debug("Достигнут конец файла");
            break;
        } else if (c_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            logger_.Error("Ошибка при чтении файла Errno: " + std::to_string(errno));
            return -1;
        } else {
            count += c_bytes;
        }
    }

    return count;
}

} // namespace wiseio
