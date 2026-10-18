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


def target(x):
    x = np.asarray(x, dtype=np.float64)
    return np.sqrt(1 + 3 * np.square(x)) + np.cos(10 * x)


def d_target(x):
    x = np.asarray(x, dtype=np.float64)
    return 3 * x / np.sqrt(1 + 3 * x**2) - 10 * np.sin(10 * x)


def d2_target(x):
    x = np.asarray(x, dtype=np.float64)
    return 3 / ((1 + 3 * x**2) ** 1.5) - 100 * np.cos(10 * x)


def main(inputf: Path):
    with inputf.open(encoding="utf-8") as f:
        grid_parameters, multiplier, coefs = collect_spline_data(f)
    spline = CubicSpline(grid_parameters, coefs)
    errs = plot(
        spline,
        (target, d_target, d2_target),
        0,
        2,
        knots=spline.knots,
        multiplier=multiplier,
        save=Path("F_spline.png"),
    )
    for name, e in zip(("f", "f'", "f''"), errs, strict=True):
        print(f"Max |{name} - S|: {e:.3e}")


if __name__ == "__main__":
    parser = build_parser()
    args = parser.parse_args()
    main(args.input)
