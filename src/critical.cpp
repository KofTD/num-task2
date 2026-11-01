#include <concepts>
#include <fstream>
#include <iostream>
#include <print>

#include <boost/program_options.hpp>
#include <grid.hpp>
#include <gsl/assert>
#include <thomas.hpp>

#include "fcos.hpp"
#include "phi.hpp"
#include "power.hpp"
#include "serialization.hpp"
#include "spline.hpp"
#include "table.hpp"

using power::cube;
using power::sqr;

namespace po = boost::program_options;
namespace sr = std::ranges;
namespace sv = std::views;

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
                Func1&& target,
                Func2&& derivative,
                Func3&& second_derivative,
                U multiplier = 10U)
{
  const grid::UniformGrid control_grid(
    grid.start(), grid.end(), grid.steps() * multiplier - 1);

  const spline::Derivative der(spline);
  const spline::SecondDerivative sder(spline);
  T max_err = 0.0;
  T max_derr = 0.0;
  T max_sderr = 0.0;
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
  }
  return { { .error = max_err, .point = xerr },
           { .error = max_derr, .point = xderr },
           { .error = max_sderr, .point = xsderr } };
}

int
main()
{
  using fp_type = double;

  fp_type start = -1.0;
  fp_type end = 1.0;
  fp_type prev_epsilon = 2.0;
  fp_type epsilon = 1.0;
  std::uint32_t phi_nodes = 100;
  Errors<fp_type> phi_errors;
  for (; std::abs(epsilon - prev_epsilon) >= 1E-9; phi_nodes++) {
    const grid::UniformGrid grid(start, end, phi_nodes);
    spline::CubicSpline phi_spline(
      grid, phi<fp_type>, phiSecondDerivative<fp_type>);
    auto errors = calculateErrors(grid,
                                  phi_spline,
                                  phi<fp_type>,
                                  phiDerivative<fp_type>,
                                  phiSecondDerivative<fp_type>);
    prev_epsilon = epsilon;
    epsilon = errors.spline.error;
    std::swap(phi_errors, errors);
  }
  std::println("Phi errors for {} nodes:", phi_nodes + 1);
  std::println(
    "Spline: {:.4} at {} (previous was {})", phi_errors.spline.error, phi_errors.spline.point, prev_epsilon);
  std::println("Derivative: {:.4} at {}",
               phi_errors.derivative.error,
               phi_errors.derivative.point);
  std::println("Second derivative: {:.4} at {}",
               phi_errors.second_derivative.error,
               phi_errors.second_derivative.point);
  std::println("");

  start = 0.0;
  end = 2.0;
  prev_epsilon = 2.0;
  epsilon = 1.0;
  std::uint32_t fcos_nodes = 100;
  Errors<fp_type> fcos_errors;
  for (; std::abs(epsilon - prev_epsilon) >= 1E-9; fcos_nodes++) {
    const grid::UniformGrid grid(start, end, fcos_nodes);
    spline::CubicSpline fcos_spline(
      grid, fcos<fp_type>, fcosSecondDerivative<fp_type>);
    auto errors = calculateErrors(grid,
                                  fcos_spline,
                                  fcos<fp_type>,
                                  fcosDerivative<fp_type>,
                                  fcosSecondDerivative<fp_type>);
    prev_epsilon = epsilon;
    epsilon = errors.spline.error;
    std::swap(fcos_errors, errors);
  }
  std::println("f + cos(10x) errors for {} nodes:", fcos_nodes + 1);
  std::println(
    "Spline: {:.4} at {} (previous was {})", fcos_errors.spline.error, fcos_errors.spline.point, prev_epsilon);
  std::println("Derivative: {:.4} at {}",
               fcos_errors.derivative.error,
               fcos_errors.derivative.point);
  std::println("Second derivative: {:.4} at {}",
               fcos_errors.second_derivative.error,
               fcos_errors.second_derivative.point);

  return 0;
}
