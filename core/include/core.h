#pragma once
#ifdef __cplusplus
#define CORE_EXTERN_C extern "C"
#else
#define CORE_EXTERN_C
#endif

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>

typedef struct stat stat_t;

CORE_EXTERN_C int wcore_o_read(const char* path);
CORE_EXTERN_C int wcore_o_write(const char* path);
CORE_EXTERN_C int wcore_o_append(const char* path);
CORE_EXTERN_C int wcore_read_and_write(const char* path);

CORE_EXTERN_C void wcore_close(int fd);

CORE_EXTERN_C ssize_t wcore_cread(
    int fd, uint8_t* buffer, size_t buffer_size, bool* is_eof, size_t* cursor);
CORE_EXTERN_C ssize_t wcore_custom_read(
    int fd, uint8_t* buffer, size_t offset, size_t buffer_size, bool* is_eof);

CORE_EXTERN_C bool wcore_awrite(
    int fd, const uint8_t* buffer, size_t buffer_size);
CORE_EXTERN_C bool wcore_cwrite(
    int fd, const uint8_t* buffer, size_t buffer_size, size_t* cursor);
CORE_EXTERN_C bool wcore_custom_write(
    int fd, const uint8_t* buffer, size_t offset, size_t buffer_size);

CORE_EXTERN_C void wcore_update_stat(int fd, stat_t* file_stat);

CORE_EXTERN_C int wcore_unlink_file(const char* file_name);
