"""2D visualisation of the best solutions.

Usage: python3 scripts/plot.py results_dir instance.csv [instance.csv ...]
Reads results_dir/best/<instance>_<method>.txt and writes results_dir/plots/<instance>_<method>.png.
"""

import math
import sys
from pathlib import Path

import matplotlib.pyplot as plt


def load(path):
    xs, ys, costs = [], [], []
    for line in Path(path).read_text().splitlines():
        if line.strip():
            x, y, c = map(int, line.split(";"))
            xs.append(x)
            ys.append(y)
            costs.append(c)
    return xs, ys, costs


def objective(xs, ys, costs, tour):
    total = sum(costs[v] for v in tour)
    for a, b in zip(tour, tour[1:] + tour[:1]):
        total += math.floor(math.hypot(xs[a] - xs[b], ys[a] - ys[b]) + 0.5)
    return total


def plot(xs, ys, costs, tour, title, out_path):
    fig, ax = plt.subplots(figsize=(10, 5.5))
    selected = set(tour)
    others = [i for i in range(len(xs)) if i not in selected]

    # Cycle edges.
    cycle = tour + tour[:1]
    ax.plot([xs[v] for v in cycle], [ys[v] for v in cycle], color="black", linewidth=0.8, zorder=1)

    # Node cost shown by colour and size; unselected nodes are faded.
    vmin, vmax = min(costs), max(costs)
    size = lambda c: 15 + 85 * (c - vmin) / (vmax - vmin or 1)
    ax.scatter([xs[i] for i in others], [ys[i] for i in others], c=[costs[i] for i in others],
               s=[size(costs[i]) for i in others], cmap="viridis", vmin=vmin, vmax=vmax,
               alpha=0.25, zorder=2)
    sc = ax.scatter([xs[i] for i in tour], [ys[i] for i in tour], c=[costs[i] for i in tour],
                    s=[size(costs[i]) for i in tour], cmap="viridis", vmin=vmin, vmax=vmax,
                    edgecolors="black", linewidths=0.5, zorder=3)
    fig.colorbar(sc, ax=ax, label="node cost", shrink=0.8)

    ax.set_title(title)
    ax.set_aspect("equal")
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    results = Path(sys.argv[1])
    out_dir = results / "plots"
    out_dir.mkdir(parents=True, exist_ok=True)

    for instance_path in sys.argv[2:]:
        name = Path(instance_path).stem
        xs, ys, costs = load(instance_path)
        for best_file in sorted((results / "best").glob(f"{name}_*.txt")):
            method = best_file.stem[len(name) + 1:]
            tour = [int(v) for v in best_file.read_text().strip().split(",")]
            obj = objective(xs, ys, costs, tour)
            out = out_dir / f"{best_file.stem}.png"
            plot(xs, ys, costs, tour, f"{name} – {method} (objective {obj})", out)
            print(f"{out}  objective={obj}")


if __name__ == "__main__":
    main()
