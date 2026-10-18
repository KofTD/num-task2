import argparse
from pathlib import Path

import numpy as np
from deserialization import collect_spline_data
from plot import plot
from spline import CubicSpline


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Plots for spline")
    parser.add_argument("--input", "-i", type=Path, required=True, help="input file")
    return parser


def phi(x):
    x = np.asarray(x, dtype=np.float64)
    return np.piecewise(
        x,
        [x < 0, x >= 0],
        [lambda t: t**3 + 3 * t**2, lambda t: -(t**3) + 3 * t**2],
    )[()]


def dphi(x):
    x = np.asarray(x, dtype=np.float64)
    return np.where(x < 0, 3 * x**2 + 6 * x, -3 * x**2 + 6 * x)[()]


def d2phi(x):
    x = np.asarray(x, dtype=np.float64)
    return np.where(x < 0, 6 * x + 6, -6 * x + 6)[()]


def main(inputf: Path):
    with inputf.open(encoding="utf-8") as f:
        grid_parameters, multiplier, coefs = collect_spline_data(f)
    spline = CubicSpline(grid_parameters, coefs)
    errs = plot(
        spline,
        (phi, dphi, d2phi),
        -1,
        1,
        knots=spline.knots,
        multiplier=multiplier,
        save=Path("phi_spline.png"),
    )
    for name, e in zip(("f", "f'", "f''"), errs, strict=True):
        print(f"Max |{name} - S|: {e:.3e}")


if __name__ == "__main__":
    parser = build_parser()
    args = parser.parse_args()
    main(args.input)
