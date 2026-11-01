#pragma once

#include <ostream>
#include <print>

#include "spline.hpp"

namespace serialization {
namespace sv = std::views;

template<typename T, std::unsigned_integral U = std::uint32_t>
void
exportSpline(std::ostream& out,
             const spline::CubicSpline<T>& spline,
             const grid::UniformGrid<T>& grid,
             U multiplier = 10U)
{
  std::println(out, "Multiplier: {}", multiplier);
  std::println(out, "Start: {}", grid.start());
  std::println(out, "End: {}", grid.end());
  std::println(out, "Nodes: {}", grid.nodes());
  for (auto&& [a, b, c, d] : sv::zip(spline.a, spline.b, spline.c, spline.d)) {
    std::println(out, "{},{},{},{}", a, b, c, d);
  }
}
} // namespace serialization
