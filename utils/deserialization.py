from typing import IO, NamedTuple


class GridParameters(NamedTuple):
    start: float
    end: float
    nodes: int


def collect_spline_data(
    inp: IO[str],
) -> tuple[GridParameters, int, list[tuple[float, float, float, float]]]:
    multiplier = int(inp.readline().split()[1])
    start = float(inp.readline().split()[1])
    end = float(inp.readline().split()[1])
    nodes = int(inp.readline().split()[1])
    grid_parameters = GridParameters(start, end, nodes)
    coefs = []
    lines = iter(inp.readline, "")
    for line in lines:
        a, b, c, d = map(float, line.split(","))
        coefs.append((a, b, c, d))
    return grid_parameters, multiplier, coefs
