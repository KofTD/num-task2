#include <cmath>
#include <concepts>

#include "power.hpp"

template<std::floating_point F>
F
undersqrt(F arg)
{
  return 1 + 3 * power::sqr(arg);
}

template<std::floating_point F>
F
targetFunc(F arg)
{
  return std::sqrt(undersqrt(arg));
}

template<std::floating_point F>
F
targetFuncDerivative(F arg)
{
  auto const s = targetFunc(arg);
  return 3 * arg / s;
}

template<std::floating_point F>
F
targetFuncSecondDerivative(F arg)
{
  auto const s = targetFunc(arg);
  return 3 / power::cube(s);
}

template<std::floating_point F>
F
fcos(F arg)
{
  return targetFunc(arg) + std::cos(10 * arg);
}

template<std::floating_point F>
F
fcosDerivative(F arg)
{
  auto const s = targetFunc(arg);
  return 3 * arg / s - 10 * std::sin(10 * arg);
}

template<std::floating_point F>
F
fcosSecondDerivative(F arg)
{
  auto const s = targetFunc(arg);
  return 3 / power::cube(s) - 100 * std::cos(10 * arg);
}
