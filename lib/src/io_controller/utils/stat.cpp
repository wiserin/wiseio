#include <cstddef>  // Copyright 2025 wiserin
#include <sys/stat.h>

#include <core.h>

#include "wise-io/stream.hpp"


namespace wiseio {

size_t Stream::GetFileSize() const {
    FdCheck();

    stat_t file_stat;
    wcore_update_stat(fd_, &file_stat);  // TODO обновлять только при изменениях

    return file_stat.st_size;
}

} // namespace wiseio
