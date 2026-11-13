#include <print>

#include <boost/program_options.hpp>
#include <grid.hpp>
#include <gsl/assert>
#include <thomas.hpp>

#include "error.hpp"
#include "f.hpp"
#include "phi.hpp"
#include "spline.hpp"

int
main()
{
  using fp_type = double;

  fp_type start = -1.0;
  fp_type end = 1.0;
  fp_type prev_epsilon = 2.0;
  fp_type epsilon = 1.0;
  fp_type epsilon_diff = 1E-9;
  std::uint32_t phi_nodes = 100;
  Errors<fp_type> phi_errors;
  for (; std::abs(epsilon - prev_epsilon) >= epsilon_diff; phi_nodes++) {
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
  std::println("Spline: {:.4} at {} (previous was {})",
               phi_errors.spline.error,
               phi_errors.spline.point,
               prev_epsilon);
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
  std::uint32_t f_nodes = 100;
  Errors<fp_type> f_errors;
  for (; std::abs(epsilon - prev_epsilon) >= epsilon_diff; f_nodes++) {
    const grid::UniformGrid grid(start, end, f_nodes);
    spline::CubicSpline f_spline(
      grid, targetFunc<fp_type>, targetFuncSecondDerivative<fp_type>);
    auto errors = calculateErrors(grid,
                                  f_spline,
                                  targetFunc<fp_type>,
                                  targetFuncDerivative<fp_type>,
                                  targetFuncSecondDerivative<fp_type>);
    prev_epsilon = epsilon;
    epsilon = errors.spline.error;
    std::swap(f_errors, errors);
  }
  std::println("f errors for {} nodes:", f_nodes + 1);
  std::println("Spline: {:.4} at {} (previous was {})",
               f_errors.spline.error,
               f_errors.spline.point,
               prev_epsilon);
  std::println("Derivative: {:.4} at {}",
               f_errors.derivative.error,
               f_errors.derivative.point);
  std::println("Second derivative: {:.4} at {}",
               f_errors.second_derivative.error,
               f_errors.second_derivative.point);
  std::println("");

  start = 0.0;
  end = 2.0;
  prev_epsilon = 2.0;
  epsilon = 1.0;
  std::uint32_t fcos_nodes = 100;
  Errors<fp_type> fcos_errors;
  for (; std::abs(epsilon - prev_epsilon) >= epsilon_diff; fcos_nodes++) {
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
  std::println("Spline: {:.4} at {} (previous was {})",
               fcos_errors.spline.error,
               fcos_errors.spline.point,
               prev_epsilon);
  std::println("Derivative: {:.4} at {}",
               fcos_errors.derivative.error,
               fcos_errors.derivative.point);
  std::println("Second derivative: {:.4} at {}",
               fcos_errors.second_derivative.error,
               fcos_errors.second_derivative.point);

  return 0;
}
