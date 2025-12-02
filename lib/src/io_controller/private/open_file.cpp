#include <cstddef>  // Copyright 2025 wiserin
#include <unistd.h>
#include <fcntl.h>

#include "wise-io/stream.hpp"
#include "wise-io/schemas.hpp"


namespace wiseio {

bool Stream::ORead(const char* path) {
    fd_ = open(path, O_RDONLY);
    if (fd_ < 0) {
        logger_.Error("Ошибка при открытии файла");
        return false;
    } else {
        logger_.Debug("Файл открыт в режиме OR");
        return true;
    }
}


bool Stream::OWrite(const char* path) {
    fd_ = open(path, O_WRONLY | O_CREAT, 0666);
    if (fd_ < 0) {
        logger_.Error("Ошибка при открытии файла");
        return false;
    } else {
        logger_.Debug("Файл открыт в режиме OW");
        return true;
    }
}


bool Stream::OAppend(const char* path) {
    fd_ = open(path, O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd_ < 0) {
        logger_.Error("Ошибка при открытии файла");
        return false;
    } else {
        logger_.Debug("Файл открыт в режиме OA");
        return true;
    }
}


bool Stream::ReadAndWrite(const char* path) {
    fd_ = open(path, O_RDWR | O_CREAT, 0666);
    if (fd_ < 0) {
        logger_.Error("Ошибка при открытии файла");
        return false;
    } else {
        logger_.Debug("Файл открыт в режиме ORDWR");
        return true;
    }
}


bool Stream::Open(const char* path) {
    switch (mode_) {
        case (OpenMode::kRead) : {
            return ORead(path);
        }
        case (OpenMode::kWrite) : {
            return OWrite(path);
        }
        case (OpenMode::kAppend) : {
            return OAppend(path);
        }
        case (OpenMode::kReadAndWrite) : {
            return ReadAndWrite(path);
        }
    }
}

} // namespace wiseio
