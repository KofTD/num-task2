from collections.abc import Iterable

import numpy as np
import numpy.typing as npt


class CubicSpline:
    def __init__(
        self,
        grid_parameters: tuple[float, float, int],
        coefs: Iterable[tuple[float, float, float, float]],
    ) -> None:
        self.splines = {}
        start, end, nodes = grid_parameters
        self.knots = np.linspace(start, end, nodes)
        self.coefs = np.asarray(coefs, dtype=np.float64)
        if self.coefs.shape != (nodes - 1, 4):
            raise ValueError("expected (a, b, c, d) row per segment")
        self.start = start
        self.end = end
        self.step = (end - start) / (nodes - 1)

    def _eval(self, x: npt.ArrayLike, order: int) -> npt.NDArray:
        nx = np.asarray(x, dtype=float)
        i = np.clip(
            np.searchsorted(self.knots, nx, side="left") - 1, 0, len(self.coefs) - 1
        )
        a, b, c, d = self.coefs[i].T
        t = nx - self.knots[i + 1]
        if order == 0:
            return a + b * t + c * t**2 / 2 + d * t**3 / 6
        if order == 1:
            return b + c * t + d * t**2 / 2
        return c + d * t

    def __call__(self, x: npt.ArrayLike) -> npt.NDArray:
        return self._eval(x, 0)

    def d1(self, x: npt.ArrayLike) -> npt.NDArray:
        return self._eval(x, 1)

    def d2(self, x: npt.ArrayLike) -> npt.NDArray:
        return self._eval(x, 2)
