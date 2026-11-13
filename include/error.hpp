#pragma once

#include "spline.hpp"
#include <concepts>
#include <grid.hpp>
#include <ranges>

template<std::floating_point F>
struct Error
{
  F error;
  F point;
};

template<std::floating_point F>
struct Errors
{
  Error<F> spline;
  Error<F> derivative;
  Error<F> second_derivative;
};

template<typename T,
         std::invocable<T> Func1,
         std::invocable<T> Func2,
         std::invocable<T> Func3,
         std::unsigned_integral U = std::uint32_t>
Errors<T>
calculateErrors(const grid::UniformGrid<T>& grid,
                const spline::CubicSpline<T>& spline,
                const Func1& target,
                const Func2& derivative,
                const Func3& second_derivative,
                U multiplier = 10U)
{
  const grid::UniformGrid control_grid(
    grid.start(), grid.end(), grid.nodes() * multiplier - 2);

  const spline::Derivative der(spline);
  const spline::SecondDerivative sder(spline);
  T max_err = 0.0;
  T max_derr = 0.0;
  T max_sderr = 0.0;
  T xerr = grid.start() - 1;
  T xderr = grid.start() - 1;
  T xsderr = grid.start() - 1;
  for (auto&& [j, knot] : std::views::enumerate(control_grid.nodeRange())) {
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
  }
  return {
    { .error = max_err, .point = xerr },
    { .error = max_derr, .point = xderr },
    { .error = max_sderr, .point = xsderr },
  };
}
