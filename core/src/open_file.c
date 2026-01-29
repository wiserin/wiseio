#include <stddef.h>  // Copyright 2025 wiserin
#include <unistd.h>
#include <fcntl.h>


int wcore_o_read(const char* path) {
    int fd = open(path, O_RDONLY);

    return fd;
}


int wcore_o_write(const char* path) {
    int fd = open(path, O_WRONLY | O_CREAT, 0666);

    return fd;
}


int wcore_o_append(const char* path) {
    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0666);

    return fd;
}


int wcore_read_and_write(const char* path) {
    int fd = open(path, O_RDWR | O_CREAT, 0666);

    return fd;
}
