"""Export ProjectileMotion as an FMU with the OpenModelica backend, run the FMU
standalone (via FMPy, independent of the Modelica compiler), and plot it.

This exercises the polaris FMU export/co-simulation path rather than the
in-process ``model.simulate`` path used by intercept.py. Supports both FMI
interface types:

    uv run python scripts/intercept_fmu.py --kind co-simulation
    uv run python scripts/intercept_fmu.py --kind model-exchange

ProjectileMotion.mo saturates the guidance command to a maximum acceleration,
which bounds the LOS-rate singularity near intercept; without that, a
fixed-step co-simulation FMU can diverge even though the adaptive
model-exchange solver tolerates it.
"""

import argparse
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401  (registers 3D projection)
from polaris import FmuKind, Model, SimulationOptions
from polaris.fmu_runtime import run_fmu

_KINDS = {
    "co-simulation": FmuKind.CO_SIMULATION,
    "model-exchange": FmuKind.MODEL_EXCHANGE,
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--kind",
        choices=sorted(_KINDS),
        default="co-simulation",
        help="FMI interface type to export and run (default: %(default)s)",
    )
    parser.add_argument(
        "--substeps",
        type=int,
        default=1,
        help=(
            "Co-simulation steps per output point (fixed-step integration only; "
            "ignored for model-exchange). Increase if a co-simulation run diverges."
        ),
    )
    return parser.parse_args()


def main():
    args = parse_args()

    # Define the model
    scripts_dir = Path(__file__).resolve().parent
    package_file = (
        scripts_dir.parents[0] / "modelica" / "ModelicaAerospace" / "package.mo"
    )
    model_file = scripts_dir / "ProjectileMotion.mo"
    model = Model(
        class_name="ProjectileMotion",
        files=(package_file, model_file),
    )

    # Export the model as an FMU of the requested kind and run it.
    build_dir = scripts_dir / "build"
    build_dir.mkdir(parents=True, exist_ok=True)
    kind = _KINDS[args.kind]
    fmu_path = model.export_fmu(build_dir, kind=kind, backend="openmodelica")

    options = SimulationOptions(stop_time=10.0, step_size=0.01)
    substeps = args.substeps if kind is FmuKind.CO_SIMULATION else 1
    result = run_fmu(fmu_path, options, substeps=substeps)

    # Extract the results
    time = result.time
    target = np.column_stack([result[f"targetPositionNED[{i}]"] for i in (1, 2, 3)])
    interceptor = np.column_stack(
        [result[f"interceptorPositionNED[{i}]"] for i in (1, 2, 3)]
    )
    rng = result["range"]
    intercepted = result["intercepted"] > 0.5

    # Index of closest approach, reported as the intercept point.
    miss_index = int(np.argmin(rng))
    miss_distance = rng[miss_index]
    miss_time = time[miss_index]

    # Setup plot
    fig = plt.figure(figsize=(11, 8), layout="constrained")

    # 3D trajectories in NED (altitude plotted as -down for readability).
    ax3d = fig.add_subplot(2, 2, 1, projection="3d")
    ax3d.plot(target[:, 0], target[:, 1], -target[:, 2], label="Target")
    ax3d.plot(
        interceptor[:, 0], interceptor[:, 1], -interceptor[:, 2], label="Interceptor"
    )
    ax3d.scatter(
        *target[miss_index, :2], -target[miss_index, 2], color="red", marker="x", s=60
    )
    ax3d.set_xlabel("North (m)")
    ax3d.set_ylabel("East (m)")
    ax3d.set_zlabel("Altitude (m)")
    ax3d.set_title("3D trajectories")
    ax3d.legend()

    # Ground track.
    ax_ground = fig.add_subplot(2, 2, 2)
    ax_ground.plot(target[:, 1], target[:, 0], label="Target")
    ax_ground.plot(interceptor[:, 1], interceptor[:, 0], label="Interceptor")
    ax_ground.set_xlabel("East (m)")
    ax_ground.set_ylabel("North (m)")
    ax_ground.set_title("Ground track")
    ax_ground.axis("equal")
    ax_ground.grid(True)
    ax_ground.legend()

    # Range to target over time.
    ax_range = fig.add_subplot(2, 2, 3)
    ax_range.plot(time, rng)
    ax_range.axvline(miss_time, color="red", linestyle="--", linewidth=1)
    ax_range.set_xlabel("time (s)")
    ax_range.set_ylabel("range (m)")
    ax_range.set_title(
        f"Closest approach: {miss_distance:.2f} m at t={miss_time:.2f} s"
    )
    ax_range.grid(True)

    # Altitude vs time for both bodies.
    ax_alt = fig.add_subplot(2, 2, 4)
    ax_alt.plot(time, -target[:, 2], label="Target")
    ax_alt.plot(time, -interceptor[:, 2], label="Interceptor")
    ax_alt.set_xlabel("time (s)")
    ax_alt.set_ylabel("altitude (m)")
    ax_alt.set_title("Altitude")
    ax_alt.grid(True)
    ax_alt.legend()

    status = "intercepted" if intercepted.any() else "missed"
    fig.suptitle(
        f"Projectile interception via FMU [{args.kind}] "
        f"({status}, miss distance {miss_distance:.2f} m)"
    )
    plt.show()


if __name__ == "__main__":
    main()
