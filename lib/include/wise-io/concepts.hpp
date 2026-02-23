#pragma once  // Copyright 2025 wiserin
#include <concepts>
#include <cstddef>
#include <type_traits>


namespace wiseio {

template <typename T>
concept Integral = std::is_integral_v<T>;


template <typename T>
concept UnsignedIntegral = Integral<T> && std::is_unsigned_v<T>;


template<typename T>
concept Hashable =
    requires(T a) {  // NOLINT
        { std::hash<T>{}(a) } -> std::convertible_to<size_t>;
        { a == a } -> std::convertible_to<bool>;
    };


} // namespace wiseio
