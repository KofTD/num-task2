#pragma once

#include <concepts>
#include <print>
#include <ranges>
#include <string>

#include "grid.hpp"
#include "spline.hpp"

namespace table {
namespace sr = std::ranges;
namespace sv = std::views;

template<sr::input_range I>
std::string
buildSeparator(I&& widths)
{
  std::string result = "+";
  for (auto&& width : widths) {
    for (int i = 0; i < width; i++) {
      result.append("-");
    }
    result.append("+");
  }
  return result;
}

template<std::integral I>
std::string
buildSeparatorFromList(std::initializer_list<I> list)
{
  return buildSeparator(list);
}

template<typename T>
void
table1(const grid::UniformGrid<T>& grid, const spline::CubicSpline<T>& spline)
{
  const auto sep = buildSeparatorFromList({ 8, 12, 12, 14, 14, 14, 14 });
  std::println("Table 1");
  std::println("{}", sep);
  std::println(
    "| {:^6} | {:^10} | {:^10} | {:^12} | {:^12} | {:^12} | {:^12} |",
    "i",
    "x_{i-1}",
    "x_i",
    "a_i",
    "b_i",
    "c_i",
    "d_i");
  std::println("{}", sep);

  for (auto&& [i, row] :
       sv::zip(grid.regionRange(), spline.a, spline.b, spline.c, spline.d) |
         sv::enumerate) {
    auto&& [region, a, b, c, d] = row;
    auto&& [xl, xr] = region;
    std::println("| {:^6} | {:>10.6f} | {:>10.6f} | {:>12.6f} | {:>12.6f} | "
                 "{:>12.6f} | {:>12.6f} |",
                 i + 1,
                 xl,
                 xr,
                 a,
                 b,
                 c,
                 d);
  }

  std::println("{}", sep);
}

template<typename T,
         std::invocable<T> Func1,
         std::invocable<T> Func2,
         std::invocable<T> Func3,
         std::unsigned_integral U = std::uint32_t>
void
table2(const grid::UniformGrid<T>& grid,
       const spline::CubicSpline<T>& spline,
       Func1&& target,
       Func2&& derivative,
       Func3&& second_derivative,
       U multiplier = 10U)
{
  const grid::UniformGrid control_grid(
    grid.start(), grid.end(), grid.steps() * multiplier - 1);

  const spline::Derivative der(spline);
  const spline::SecondDerivative sder(spline);

  const auto sep2 =
    buildSeparatorFromList({ 6, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 });
  std::println("Table 2");
  std::println("{}", sep2);
  std::println(
    "| {:^4} | {:^12} | {:^12} | {:^12} | {:^12} | {:^12} | {:^12} | {:^12} "
    "| {:^12} | {:^12} | {:^12} |",
    "j",
    "x_j",
    "F(x_j)",
    "S(x_j)",
    "F-S",
    "F'(x_j)",
    "S'(x_j)",
    "F'-S'",
    "F''(x_j)",
    "S''(x_j)",
    "F''-S''");
  std::println("{}", sep2);

  T max_err = std::numeric_limits<T>::min();
  T max_derr = std::numeric_limits<T>::min();
  T max_sderr = std::numeric_limits<T>::min();
  T xerr = grid.start() - 1;
  T xderr = grid.start() - 1;
  T xsderr = grid.start() - 1;

  for (auto&& [j, knot] : sv::enumerate(control_grid.nodeRange())) {
    auto target_val = std::invoke(target, knot);
    auto spline_val = spline(knot);
    auto err = target_val - spline_val;

    auto tder_val = std::invoke(derivative, knot);
    auto der_val = der(knot);
    auto der_err = tder_val - der_val;

    auto tsder_val = std::invoke(second_derivative, knot);
    auto sder_val = sder(knot);
    auto sder_err = tsder_val - sder_val;

    if (auto aerr = std::abs(err); max_err < aerr) {
      xerr = knot;
      max_err = aerr;
    }
    if (auto aerr = std::abs(der_err); max_derr < aerr) {
      xderr = knot;
      max_derr = aerr;
    }
    if (auto aerr = std::abs(sder_err); max_sderr < aerr) {
      xsderr = knot;
      max_sderr = aerr;
    }

    std::println("| {:>4} | {:>12.6f} | {:>12.6f} | {:>12.6f} | {:>12.4e} "
                 "| {:>12.6f} | {:>12.6f} | {:>12.4e} "
                 "| {:>12.6f} | {:>12.6f} | {:>12.4e} |",
                 j,
                 knot,
                 target_val,
                 spline_val,
                 err,
                 tder_val,
                 der_val,
                 der_err,
                 tsder_val,
                 sder_val,
                 sder_err);
  }

  std::println("{}", sep2);
  std::println("max |F-S|     = {:.4e} at x = {}", max_err, xerr);
  std::println("max |F'-S'|   = {:.4e} at x = {}", max_derr, xderr);
  std::println("max |F''-S''| = {:.5e} at x = {}", max_sderr, xsderr);
}
} // namespace table
