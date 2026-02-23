// NOLINTBEGIN  Copyright 2025 wiserin
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>


ssize_t wcore_cread(
        int fd, uint8_t* buffer, size_t buffer_size, bool* is_eof, size_t* cursor) {

    size_t count = 0;

    while (count < buffer_size) {
        ssize_t c_bytes = pread(fd, buffer + count, buffer_size - count, *cursor + count);

        if (c_bytes + count >= buffer_size) {
            count += c_bytes;
            break;
        } else if (c_bytes == 0) {
            *is_eof = true;
            break;
        } else if (c_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("WiseIO core error (wcore_read). Errno: %d", errno);
            return -1;
        } else {
            count += c_bytes;
        }
    }
    *cursor += count;

    return count;
}


ssize_t wcore_custom_read(
        int fd, uint8_t* buffer, size_t offset, size_t buffer_size, bool* is_eof) {

    size_t count = 0;

    while (count < buffer_size) {
        ssize_t c_bytes = pread(fd, buffer + count, buffer_size - count, offset + count);

        if (c_bytes + count >= buffer_size) {
            count += c_bytes;
            break;
        } else if (c_bytes == 0) {
            *is_eof = true;
            break;
        } else if (c_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("WiseIO core error (wcore_read). Errno: %d", errno);
            return -1;
        } else {
            count += c_bytes;
        }
    }

    return count;
}
// NOLINTEND
