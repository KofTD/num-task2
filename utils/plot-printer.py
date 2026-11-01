"""Monochrome, print-friendly spline plots.

Every quantity gets its own panel (no overlays, no colour):
    1) target  F^(k)(x)
    2) spline  S^(k)(x)   (knots marked with small open circles)
    3) error   |F^(k)(x) - S^(k)(x)|
One A4 page (PNG + PDF) per derivative order k = 0, 1, 2.

Usage:
    python plot-printer.py -i spline.txt -p a        # phi(x), [-1, 1]
    python plot-printer.py -i spline.txt -p b        # sqrt(1+3x^2), [0, 2]
    python plot-printer.py -i spline.txt -p c        # sqrt(1+3x^2)+cos(10x), [0, 2]
"""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from deserialization import collect_spline_data
from spline import CubicSpline

# ---------------------------------------------------------------- targets


def _arr(x):
    return np.asarray(x, dtype=np.float64)


def phi(x):
    x = _arr(x)
    return np.where(x < 0, x**3 + 3 * x**2, -(x**3) + 3 * x**2)[()]


def dphi(x):
    x = _arr(x)
    return np.where(x < 0, 3 * x**2 + 6 * x, -3 * x**2 + 6 * x)[()]


def d2phi(x):
    x = _arr(x)
    return np.where(x < 0, 6 * x + 6, -6 * x + 6)[()]


def f(x):
    return np.sqrt(1 + 3 * _arr(x) ** 2)


def df(x):
    x = _arr(x)
    return 3 * x / np.sqrt(1 + 3 * x**2)


def d2f(x):
    x = _arr(x)
    return 3 / (1 + 3 * x**2) ** 1.5


def fc(x):
    return f(x) + np.cos(10 * _arr(x))


def dfc(x):
    return df(x) - 10 * np.sin(10 * _arr(x))


def d2fc(x):
    return d2f(x) - 100 * np.cos(10 * _arr(x))


# name -> (label, (F, F', F''))
PROBLEMS = {
    "a": ("\\varphi", (phi, dphi, d2phi)),
    "b": ("f", (f, df, d2f)),
    "c": ("\\sqrt{1 + 3x^{2}}+\\cos 10x", (fc, dfc, d2fc)),
}

# ---------------------------------------------------------------- plotting

PAD = 0.05  # fraction of the data range left empty around every plot

PRIMES = ("", "'", "''")
SUFFIXES = ("", "_d1", "_d2")


def plot_order(spline, F, order, label, x, knots, out: Path):
    p = PRIMES[order]
    y_true, y_spl = F(x), spline_eval(spline, order, x)
    err = y_true - y_spl

    fig, (ax_f, ax_s, ax_e) = plt.subplots(3, 1, figsize=(8.27, 11.69))

    ax_f.plot(x, y_true, "k-", lw=1.4)
    ax_f.set_title(f"$F{p}(x)$,  $F={label}$")

    ax_s.plot(x, y_spl, "k-", lw=1.4)
    ax_s.plot(
        knots,
        spline_eval(spline, order, knots),
        "o",
        mfc="white",
        mec="black",
        ms=4,
        ls="none",
    )
    ax_s.set_title(f"$S{p}(x)$  (knots: $\\circ$)")

    # same y-range for F and S so they are visually comparable
    lo = min(y_true.min(), y_spl.min())
    hi = max(y_true.max(), y_spl.max())
    pad = PAD * (hi - lo or 1.0)
    for ax in (ax_f, ax_s):
        ax.set_ylim(lo - pad, hi + pad)

    aerr = np.abs(err)
    ax_e.plot(x, aerr, "k-", lw=1.0)
    for k in knots:
        ax_e.axvline(k, color="black", lw=0.3, ls=":")
    top = aerr.max() or 1.0
    ax_e.set_ylim(-PAD * top, (1 + PAD) * top)
    ax_e.set_title(f"$|F{p}(x) - S{p}(x)|$,  max = {aerr.max():.3e}")
    ax_e.ticklabel_format(axis="y", style="sci", scilimits=(-2, 2))

    for ax in (ax_f, ax_s, ax_e):
        ax.set_xlim(x[0] - PAD * (x[-1] - x[0]), x[-1] + PAD * (x[-1] - x[0]))
        ax.set_xlabel("x")
        ax.grid(True, color="black", alpha=0.15, lw=0.5)
        for side in ("top", "right"):
            ax.spines[side].set_visible(False)

    fig.tight_layout()
    fig.savefig(out.with_suffix(".png"), dpi=200)
    fig.savefig(out.with_suffix(".pdf"))
    return float(np.abs(err).max())


def spline_eval(spline, order, x):
    return (spline, spline.d1, spline.d2)[order](x)


# ---------------------------------------------------------------- main


def main():
    ap = argparse.ArgumentParser(description="Monochrome spline plots for printing")
    ap.add_argument("--input", "-i", type=Path, required=True, help="input file")
    ap.add_argument("--problem", "-p", choices=PROBLEMS, required=True)
    ap.add_argument("--outdir", "-o", type=Path, default=Path("."))
    ap.add_argument("--show", action="store_true", help="also open the windows")
    args = ap.parse_args()

    with args.input.open(encoding="utf-8") as fh:
        grid, multiplier, coefs = collect_spline_data(fh)
    spline = CubicSpline(grid, coefs)

    label, targets = PROBLEMS[args.problem]
    x = np.linspace(
        grid.start, grid.end, max((len(spline.knots) - 1) * multiplier + 1, 1000)
    )

    args.outdir.mkdir(parents=True, exist_ok=True)
    for order, F in enumerate(targets):
        out = args.outdir / f"{args.problem}_spline{SUFFIXES[order]}"
        e = plot_order(spline, F, order, label, x, spline.knots, out)
        print(f"Max |F{PRIMES[order]} - S{PRIMES[order]}|: {e:.3e}  -> {out}.png/.pdf")

    if args.show:
        plt.show()


if __name__ == "__main__":
    main()
