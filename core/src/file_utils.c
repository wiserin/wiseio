// NOLINTBEGIN  Copyright 2025 wiserin
#include <core.h>
#include <unistd.h>


int wcore_unlink_file(const char* file_name) {
    return unlink(file_name);
}
// NOLINTEND