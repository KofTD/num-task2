#include <concepts>

#include "power.hpp"

template<std::floating_point F>
F
phi(F val)
{
  if (val <= 0)
    return power::cube(val) + 3 * power::sqr(val);
  return -power::cube(val) + 3 * power::sqr(val);
}

template<std::floating_point F>
F
phiDerivative(F val)
{
  if (val <= 0)
    return 3 * power::sqr(val) + 6 * val;
  return -3 * power::sqr(val) + 6 * val;
}

template<std::floating_point F>
F
phiSecondDerivative(F val)
{
  if (val <= 0)
    return 6 * val + 6;
  return -6 * val + 6;
}
