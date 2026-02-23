#pragma once  // Copyright 2025 wiserin
#include <string>
#include <utility>

#include "wise-io/byte/chunks.hpp"
#include "wise-io/byte/bytefile.hpp"
#include "wise-io/concepts.hpp"


using str = std::string;


namespace wiseio {

template <Hashable T>
ByteFile<T>::ByteFile(const char* file_name)
        : file_engine_(ByteFileEngine(file_name)) {}


template <Hashable T>
void ByteFile<T>::AddChunk(std::unique_ptr<BaseChunk> chunk, T&& name) {
    if (IsNameInIndex(name)) {
        throw std::logic_error("Имя уже занято");
    }
    index_[std::move(name)] = chunk.get();
    layout_.push_back(std::move(chunk));
}


template <Hashable T>
void ByteFile<T>::AddChunk(std::unique_ptr<BaseChunk> chunk, const T& name) {
    if (IsNameInIndex(name)) {
        throw std::logic_error("Имя уже занято");
    }
    index_[name] = chunk.get();
    layout_.push_back(std::move(chunk));
}


template <Hashable T>
BaseChunk& ByteFile<T>::GetChunk(const T& name) {
    if (!IsNameInIndex(name)) {
        throw std::logic_error("Чанк с таким именем не найден");
    }
    return *index_[name];
}


template <Hashable T>
BaseChunk& ByteFile<T>::GetAndLoadChunk(const T& name) {
    if (!IsNameInIndex(name)) {
        throw std::logic_error("Чанк с таким именем не найден");
    }
    BaseChunk& chunk = *index_[name];
    file_engine_.ReadChunk(chunk);
    return chunk;
}


template <Hashable T>
bool ByteFile<T>::IsNameInIndex(const T& name) {
    return index_.contains(name);
}


template <Hashable T>
void ByteFile<T>::InitChunksFromFile() {
    file_engine_.InitChunks(layout_);
}


template <Hashable T>
void ByteFile<T>::Compile() {
    file_engine_.CompileFile(layout_);
}


} // namespace wiseio
