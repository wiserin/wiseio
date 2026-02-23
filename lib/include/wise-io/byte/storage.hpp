#pragma once  // Copyright 2025 wiserin
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <wise-io/stream.hpp>


using str = std::string;


namespace wiseio {

class Storage {
    std::vector<uint8_t> data_;  // TODO переписать на shared ptr
    StorageState state_ = StorageState::kClean;
    Stream stream_;

    inline static std::filesystem::path cache_dir = "";

    void ReadFromCache();

 public:
    Storage() = default;
    Storage(const Storage& another) = delete;
    Storage& operator=(const Storage& another) = delete;
    Storage(Storage&& another) noexcept = default;
    Storage& operator=(Storage&& another) noexcept = default;

    static void SetCacheDir(str&& path);
    void Commit();

    [[nodiscard]] std::vector<uint8_t>& GetData();

    [[nodiscard]] bool IsChanged();

    ~Storage() = default;
};

} // namespace wiseio
