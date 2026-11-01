#pragma once

#include <concepts>

namespace power {
namespace impl {

template<typename T>
concept Number = std::integral<T> or std::floating_point<T>;

} // namespace impl

template<impl::Number N>
constexpr N
sqr(N base)
{
  return base * base;
}

template<impl::Number N>
constexpr N
cube(N base)
{
  return base * base * base;
}

} // namespace power
