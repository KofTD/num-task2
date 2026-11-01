#include <fstream>
#include <iostream>
#include <print>

#include <boost/program_options.hpp>
#include <grid.hpp>
#include <gsl/assert>
#include <thomas.hpp>

#include "fcos.hpp"
#include "power.hpp"
#include "serialization.hpp"
#include "spline.hpp"
#include "table.hpp"

using power::cube;
using power::sqr;

namespace po = boost::program_options;
namespace sr = std::ranges;
namespace sv = std::views;

int
main(int argc, char* argv[])
{
  std::string output_name;
  std::uint32_t nodes = 1;
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "this help message")(
    "nodes,n",
    po::value<std::uint32_t>(&nodes),
    "number of nodes between start and end")(
    "output,o",
    po::value<std::string>(&output_name)->default_value("spline-out-c.txt"),
    "output for coefficients and grid");
  po::variables_map vmap;
  po::store(po::parse_command_line(argc, argv, desc), vmap);
  po::notify(vmap);

  if (vmap.contains("help") or vmap.empty()) {
    std::cout << desc << '\n';
    return 0;
  }

  if (!vmap.contains("nodes")) {
    std::println("Requires number of nodes! (see help)");
    return 1;
  }

  using fp_type = double;

  fp_type start = 0.0;
  fp_type end = 2.0;
  const grid::UniformGrid grid(start, end, nodes);

  spline::CubicSpline spline(grid, fcos<fp_type>, fcosSecondDerivative<fp_type>);

  table::table1(grid, spline);
  std::println("");
  table::table2(grid,
                spline,
                fcos<fp_type>,
                fcosDerivative<fp_type>,
                fcosSecondDerivative<fp_type>);

  std::ofstream spline_out(output_name);
  if (!spline_out.is_open()) {
    std::println("Error: could not open {} to dump coefs", output_name);
    return 1;
  }
  serialization::exportSpline(spline_out, spline, grid);
  spline_out.close();

  return 0;
}
