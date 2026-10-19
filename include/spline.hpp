#pragma once

#include <concepts>
#include <functional>
#include <ranges>
#include <vector>

#include <thomas.hpp>

#include "grid.hpp"
#include "power.hpp"

namespace spline {
namespace sr = std::ranges;
namespace sv = std::views;

template <std::floating_point F>
auto createStrips(const grid::UniformGrid<F> &grid)
    -> std::tuple<std::vector<F>, std::vector<F>, std::vector<F>> {
    const F step = grid.step();
    const std::size_t n = grid.steps() - 1;
    return {
        std::vector<F>(n, -step),     // a
        std::vector<F>(n, -step),     // b
        std::vector<F>(n, 4 * step),  // c
    };
}

template <std::floating_point F, std::invocable<F> Func>
auto createPhis(const grid::UniformGrid<F> &grid, Func &&target, Func &&second_derivative)
    -> std::vector<F> {
    std::vector<F> phis;
    phis.reserve(grid.nodes());
    const F step = grid.step();
    phis.push_back(std::invoke(std::forward<Func>(second_derivative), grid.start()));  // mu1
    auto phibi = std::back_inserter(phis);

    sr::move(
        grid.nodeRange() | sv::transform(std::forward<Func>(target))
            | sv::adjacent<3> | sv::transform([step](std::tuple<F, F, F> &&vals) {
                  auto &&[fpred, fcur, fpost] = vals;
                  return 6 * (fpost - 2 * fcur + fpred) / step;
              }),
        phibi);
    // for (auto &&[fpred, fcur, fpost] :
    //      grid.nodeRange() | sv::transform(std::forward<Func>(target)) | sv::adjacent<3>) {
    //     phis.push_back(6 * (fpost - 2 * fcur + fpred) / step);
    // }
    phis.push_back(std::invoke(std::forward<Func>(second_derivative), grid.end()));  // mu2
    return phis;
}

template <std::floating_point F, std::invocable<F> Func>
auto calcA(const grid::UniformGrid<F> &grid, Func &&target) -> std::vector<F> {
    return grid.nodeRange() | sv::drop(1) | sv::transform(std::forward<Func>(target))
         | sr::to<std::vector>();
}

template <std::floating_point F, sr::forward_range C>
    requires std::convertible_to<sr::range_value_t<C>, F>
auto calcD(C &&c_coefs, F step) -> std::vector<F> {
    return sv::pairwise(std::forward<C>(c_coefs))
         | sv::transform([step](std::tuple<F, F> tup) -> F {
               auto [cprev, ccur] = tup;
               return (ccur - cprev) / step;
           })
         | sr::to<std::vector>();
}

template <std::floating_point F, sr::forward_range C, std::invocable<F> Func>
    requires std::convertible_to<sr::range_value_t<C>, F>
auto calcB(const grid::UniformGrid<F> &grid, C &&c_coefs, Func &&target) {
    auto fvals = sv::transform(grid.nodeRange(), std::forward<Func>(target));
    return sv::zip(sv::pairwise(fvals), sv::pairwise(std::forward<C>(c_coefs)))
         | sv::transform([&grid](auto vals) -> F {
               auto [fvals, cvals] = vals;
               auto [fpred, fcur] = fvals;
               auto [cpred, ccur] = cvals;
               F step = grid.step();
               return (fcur - fpred) / step + ccur * step / 3 + cpred * step / 6;
           })
         | sr::to<std::vector>();
}

template <std::floating_point F>
struct Knots {
    std::vector<F> knots;

    template <sr::input_range I>
        requires std::convertible_to<sr::range_value_t<I>, F>
    explicit Knots(I &&knots) : knots(std::from_range, std::forward<I>(knots)) {}

    auto findPos(F val) const -> std::size_t {
        F lborder = this->knots.front();
        F rborder = this->knots.back();
        if (val < lborder or val > rborder) {
            auto msg = std::format(
                "Spline is not defined at {}. Our bounds are [{}, {}]", val, lborder, rborder);
            throw std::invalid_argument(msg);
        }
        auto lb = sr::lower_bound(knots, val);
        std::size_t lknot = sr::distance(knots.begin(), lb);
        return lknot;
    }
};

template <std::floating_point F>
struct CubicSpline {
    std::vector<F> a;
    std::vector<F> b;
    std::vector<F> c;
    std::vector<F> d;
    Knots<F> knots;

    template <std::invocable<F> Target, std::invocable<F> SecondDerivative>
    CubicSpline(
        const grid::UniformGrid<F> &grid, Target &&target, SecondDerivative &&second_derivative)
        : knots(grid.nodeRange()) {
        auto [stripa, stripb, stripc] = createStrips(grid);
        auto phis = createPhis(grid, target, second_derivative);
        auto c_coefs =
            thomas::thomas(stripa, stripb, stripc, phis, static_cast<F>(0), static_cast<F>(0));
        auto a_coefs = calcA(grid, target);
        auto d_coefs = calcD(c_coefs, grid.step());
        auto b_coefs = calcB(grid, c_coefs, target);

        this->a = std::move(a_coefs);
        this->b = std::move(b_coefs);
        this->c.assign_range(c_coefs | sv::drop(1));  // 1st one is fake
        this->d = std::move(d_coefs);
    }

    auto operator()(F val) const -> F {
        const std::size_t k = knots.findPos(val);
        const std::size_t index = k == 0 ? 0 : k - 1;  // interval [x_i, x_{i+1}]
        F a = this->a[index];
        F b = this->b[index];
        F c = this->c[index];
        F d = this->d[index];
        F xi = this->knots.knots[index + 1];
        F t = val - xi;

        return a + b * t + c * power::sqr(t) / 2 + d * power::cube(t) / 6;
    }
};

template <std::floating_point F>
struct Derivative {
    std::vector<F> b;
    std::vector<F> c;
    std::vector<F> d;
    Knots<F> knots;

    explicit Derivative(CubicSpline<F> &&spline)
        : b(std::move(spline.b))
        , c(std::move(spline.c))
        , d(std::move(spline.d))
        , knots(std::move(spline.knots)) {}

    explicit Derivative(const CubicSpline<F> &spline) : Derivative(CubicSpline(spline)) {}

    auto operator()(F val) const -> F {
        const std::size_t k = knots.findPos(val);
        const std::size_t index = k == 0 ? 0 : k - 1;  // interval [x_i, x_{i+1}]
        F b = this->b[index];
        F c = this->c[index];
        F d = this->d[index];
        F xi = this->knots.knots[index + 1];
        F t = val - xi;
        return b + c * t + d * power::sqr(t) / 2;
    }
};

template <std::floating_point F>
struct SecondDerivative {
    std::vector<F> c;
    std::vector<F> d;
    Knots<F> knots;

    explicit SecondDerivative(CubicSpline<F> &&spline)
        : c(std::move(spline.c))
        , d(std::move(spline.d))
        , knots(std::move(spline.knots)) {}

    explicit SecondDerivative(const CubicSpline<F> &spline)
        : SecondDerivative(CubicSpline(spline)) {}

    auto operator()(F val) const -> F {
        const std::size_t k = knots.findPos(val);
        const std::size_t index = k == 0 ? 0 : k - 1;  // interval [x_i, x_{i+1}]
        F c = this->c[index];
        F d = this->d[index];
        F xi = this->knots.knots[index + 1];

        return c + d * (val - xi);
    }
};

}  // namespace spline
