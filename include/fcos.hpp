#include <cmath>
#include <concepts>

#include "power.hpp"

template<std::floating_point F>
F
undersqrt(F val)
{
  return 1 + 3 * power::sqr(val);
}

template<std::floating_point F>
F
fcos(F val)
{
  return std::sqrt(undersqrt(val)) + std::cos(10 * val);
}

template<std::floating_point F>
F
fcosDerivative(F val)
{
  F s = std::sqrt(undersqrt(val));
  return 3 * val / s - 10 * std::sin(10 * val);
}

template<std::floating_point F>
F
fcosSecondDerivative(F val)
{
  F s = std::sqrt(undersqrt(val));
  return 3 / power::cube(s) - 100 * std::cos(10 * val);
}
