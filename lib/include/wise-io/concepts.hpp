#pragma once  // Copyright 2025 wiserin
#include <type_traits>
#include <concepts>
#include <cstddef>


namespace wiseio {

template <typename T>
concept Integral = std::is_integral_v<T>;


template <typename T>
concept UnsignedIntegral = Integral<T> && std::is_unsigned_v<T>;


template<typename T>
concept Hashable =
    requires(T a) {
        { std::hash<T>{}(a) } -> std::convertible_to<size_t>;
        { a == a } -> std::convertible_to<bool>;
    };


} // namespace wiseio
