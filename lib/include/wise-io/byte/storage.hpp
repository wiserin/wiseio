#pragma once  // Copyright 2025 wiserin
#include <cstdint>
#include <vector>
#include <string>

#include <wise-io/stream.hpp>


using str = std::string;


namespace wiseio {


class Storage {
    std::vector<uint8_t> data_;
    StorageState state_ = StorageState::kClean;
    str file_name_;

 public:
    void Commit();
    std::vector<uint8_t>& GetData();

    bool IsChanged();
};

} // namespace wiseio
