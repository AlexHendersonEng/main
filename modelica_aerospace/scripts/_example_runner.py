"""Shared Polaris simulation and plotting support for the example scripts."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt
from polaris import Model, SimulationOptions

PROJECT_ROOT = Path(__file__).resolve().parents[1]
PACKAGE_ROOT = PROJECT_ROOT / "ModelicaAerospace"
PACKAGE_FILE = PACKAGE_ROOT / "package.mo"
PLOT_DIR = PROJECT_ROOT / "scripts" / "plots"


@dataclass(frozen=True)
class PlotPanel:
    title: str
    x_label: str
    y_label: str
    series: tuple[tuple[str, str], ...]
    x_variable: str = "time"


def run_example(
    class_name: str,
    panels: tuple[PlotPanel, ...],
    *,
    stop_time: float,
    step_size: float,
    tolerance: float = 1e-8,
) -> None:
    # Parse input arguments
    parser = argparse.ArgumentParser(description=f"Simulate and plot {class_name}.")
    parser.add_argument(
        "--backend",
        choices=("openmodelica", "rumoca"),
        default="openmodelica",
        help="Modelica compiler backend (default: openmodelica)",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=PLOT_DIR,
        help="directory for the generated PNG (default: plots)",
    )
    parser.add_argument(
        "--show", default=True, action="store_true", help="show the plot after saving it"
    )
    args = parser.parse_args()

    if args.backend == "rumoca":
        # Rumoca needs the package parent and sibling sources to resolve nested classes.
        sources = sorted(path for path in PACKAGE_ROOT.rglob("*.mo") if path != PACKAGE_FILE)
        files = (PACKAGE_FILE, PACKAGE_ROOT.parent, *sources)
    else:
        files = (PACKAGE_FILE,)

    # Create plots directory
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / ".gitignore").write_text("*")

    # Simulate the model
    model = Model(
        f"ModelicaAerospace.Examples.{class_name}",
        files=files,
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(
            stop_time=stop_time,
            step_size=step_size,
            tolerance=tolerance,
        ),
        backend=args.backend,
    )

    # Setup plotting environment
    if not args.show:
        matplotlib.use("Agg")

    # Setup figure
    figure, axes = plt.subplots(
        len(panels),
        1,
        figsize=(10, max(3.2, 2.8 * len(panels))),
        squeeze=False,
    )

    # Plot each panel
    for axis, panel in zip(axes[:, 0], panels, strict=True):
        x_values = result[panel.x_variable]

        for variable, label in panel.series:
            axis.plot(x_values, result[variable], label=label)

        axis.set_title(panel.title)
        axis.set_xlabel(panel.x_label)
        axis.set_ylabel(panel.y_label)
        axis.grid(True, alpha=0.3)

        if len(panel.series) > 1:
            axis.legend()

    figure.suptitle(f"ModelicaAerospace.Examples.{class_name}")
    figure.tight_layout()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    output_path = args.output_dir / f"{class_name}.png"
    figure.savefig(output_path, dpi=150)

    if args.show:
        plt.show()

    plt.close(figure)

    print(f"Saved plot to {output_path}")
