#include <cstddef>
#include <sys/stat.h> // Copyright 2025 wiserin


#include "wise-io/stream.hpp"



namespace wiseio {

void Stream::UpdateStat(stat_t& file_stat) const {
    fstat(fd_, &file_stat);
}


size_t Stream::GetFileSize() const {
    FdCheck();

    stat_t file_stat;
    UpdateStat(file_stat);

    return file_stat.st_size;
}

} // namespace wiseio