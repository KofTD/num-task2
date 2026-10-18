#pragma once

#include <concepts>

namespace power {
namespace impl {

template <typename T>
concept Number = std::integral<T> or std::floating_point<T>;

}  // namespace impl

template <impl::Number N>
constexpr auto sqr(N base) -> N {
    return base * base;
}

template <impl::Number N>
constexpr auto cube(N base) -> N {
    return base * base * base;
}

}  // namespace power
