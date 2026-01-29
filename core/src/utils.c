#include "core.h"


void wcore_update_stat(int fd, stat_t* file_stat) {
    fstat(fd, file_stat);
}