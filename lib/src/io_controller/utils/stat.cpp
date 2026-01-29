#include <cstddef>
#include <sys/stat.h> // Copyright 2025 wiserin

#include <core.h>

#include "wise-io/stream.hpp"


namespace wiseio {

size_t Stream::GetFileSize() const {
    FdCheck();

    stat_t file_stat;
    wcore_update_stat(fd_, &file_stat);

    return file_stat.st_size;
}

} // namespace wiseio
