#include <cstddef>  // Copyright 2025 wiserin
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <string>

#include <wise-io/byte/storage.hpp>
#include "wise-io/schemas.hpp"


using str = std::string;

namespace wiseio {

std::vector<uint8_t>& Storage::GetData() {
    state_ = StorageState::kDirty;
    return data_;
}


bool Storage::IsChanged() {
    return state_ != StorageState::kClean;
}


} // namespace wiseio
