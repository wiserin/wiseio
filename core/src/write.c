#include <stddef.h>  // Copyright 2025 wiserin
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>


bool wcore_awrite(
        int fd, const uint8_t* buffer, size_t buffer_size) {

    size_t written = 0;

    while (written < buffer_size) {
        ssize_t res = write(fd, buffer + written, buffer_size - written);

        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("WiseIO core error (wcore_read). Errno: %d", errno);
            return false;
        }
        written += res;
    }

    return true;
}


bool wcore_cwrite(
        int fd, const uint8_t* buffer, size_t buffer_size, size_t* cursor) {

    size_t written = 0;

    while (written < buffer_size) {
        ssize_t res = pwrite(fd, buffer + written, buffer_size - written, *cursor + written);

        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("WiseIO core error (wcore_read). Errno: %d", errno);
            return false;
        }
        written += res;
    }

    *cursor += written;

    return true;
}


bool wcore_custom_write(
        int fd, const uint8_t* buffer, size_t offset, size_t buffer_size) {

    size_t written = 0;

    while (written < buffer_size) {
        ssize_t res = pwrite(fd, buffer + written, buffer_size - written, offset + written);

        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("WiseIO core error (wcore_read). Errno: %d", errno);
            return false;
        }
        written += res;
    }

    return true;
}
