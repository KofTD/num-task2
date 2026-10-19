from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

NAMES = ("f", "f'", "f''")
SUFFIXES = ("", "_d1", "_d2")


def plot(
    spline, targets, start, end, knots, multiplier, save: Path | None = None
) -> tuple[float, float, float]:
    x = np.linspace(start, end, max((len(knots) - 1) * multiplier + 1, 1000))
    funcs = (spline, spline.d1, spline.d2)

    max_errs = []
    for S, F, name, suffix in zip(funcs, targets, NAMES, SUFFIXES):
        y_true, y_spl = F(x), S(x)

        fig, (top, bottom) = plt.subplots(
            2, 1, figsize=(9, 7), sharex=True, gridspec_kw={"height_ratios": [3, 1]}
        )

        top.plot(x, y_true, "k-", lw=2, label=f"target {name}(x)")
        top.plot(x, y_spl, "r--", lw=1.5, label=f"spline S{name[1:]}(x)")
        top.plot(knots, F(knots), "bo", ms=5, label="knots")
        top.set_ylabel("y")
        top.legend()
        top.grid(alpha=0.3)

        err = np.abs(y_true - y_spl)
        bottom.semilogy(x, np.maximum(err, 1e-18), "g-")  # floor avoids log(0)
        for k in knots:
            bottom.axvline(k, color="gray", lw=0.5, alpha=0.4)
        bottom.set_xlabel("x")
        bottom.set_ylabel(f"|{name} - S{name[1:]}|")
        bottom.grid(alpha=0.3, which="both")

        fig.tight_layout()
        if save:
            fig.savefig(save.with_name(f"{save.stem}{suffix}{save.suffix}"), dpi=150)
        max_errs.append(float(err.max()))

    plt.show()  # one call opens all three windows
    return tuple(max_errs)  # ty: ignore[invalid-return-type]
