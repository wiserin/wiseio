#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
#include <string>

#include "wise-io/byte/storage.hpp"
#include "wise-io/schemas.hpp"
#include "wise-io/utils.hpp"


using str = std::string;

namespace wiseio {

std::vector<uint8_t>& Storage::GetData() {
    if (state_ == StorageState::kCommited) {
        ReadFromCache();
    }
    state_ = StorageState::kDirty;
    return data_;
}


void Storage::ReadFromCache() {
    stream_.ReadAll(data_);
}


bool Storage::IsChanged() {
    return state_ != StorageState::kClean;
}


void Storage::SetCacheDir(str&& path) {
    if (!std::filesystem::is_directory(path)) {
        throw std::runtime_error("Unknown dir");
    }
    cache_dir = std::move(path);
}


void Storage::Commit() {
    if (stream_.IsOpen()) {
        stream_.Close();
    }
    stream_ = CreateStream(cache_dir / FileNamer::GetName(), OpenMode::kReadAndWrite, true);

    stream_.CWrite(data_);

    std::vector<uint8_t>().swap(data_);

    state_ = StorageState::kCommited;
}

}