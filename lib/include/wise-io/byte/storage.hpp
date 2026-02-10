#pragma once  // Copyright 2025 wiserin
#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>

#include <wise-io/stream.hpp>


using str = std::string;


namespace wiseio {

class Storage {
    std::vector<uint8_t> data_;
    StorageState state_ = StorageState::kClean;
    Stream stream_;

    inline static std::filesystem::path cache_dir = "";

    void ReadFromCache();

 public:
    static void SetCacheDir(str&& path);
    void Commit();
    std::vector<uint8_t>& GetData();

    bool IsChanged();
};

} // namespace wiseio
