"""Showcase of Polaris so far, using a mass-spring-damper model.

Run from the polaris directory:  uv run python scripts/showcase.py [openmodelica|rumoca]

Plots are written to scripts/output/. Without an argument every installed backend is used.
"""

from __future__ import annotations

import sys
from pathlib import Path

from polaris import Model, Result, SimulationOptions
from polaris.backends import available_backends, get_backend
from polaris.plotting import compare, plot

HERE = Path(__file__).parent
OUT = HERE / "output"


def main() -> None:
    OUT.mkdir(exist_ok=True)

    # 1. Which backends exist on this machine?
    backends = [sys.argv[1]] if len(sys.argv) > 1 else available_backends()
    for name in backends:
        be = get_backend(name)
        print(f"backend {name}: version {be.version()}, capabilities {be.capabilities}")

    model = Model("MassSpringDamper", [HERE / "MassSpringDamper.mo"])
    options = SimulationOptions(stop_time=5.0, step_size=0.01)

    # 2. Simulate the same model on every backend and plot the results.
    results: dict[str, Result] = {}
    for name in backends:
        results[name] = model.simulate(options, backend=name)
        r = results[name]
        print(f"{name}: x(5) = {r['x'][-1]:.5f}, {len(r.time)} points")
        plot(
            r,
            ["x", "v"],
            separate=True,
            title=f"MassSpringDamper ({name})",
            save_to=OUT / f"msd_{name}.png",
        )

    # 3. Cross-backend comparison of the position.
    if len(results) > 1:
        compare(results, "x", title="Position by backend", save_to=OUT / "backend_compare.png")

    # 4. Parameter overrides without editing the model: sweep damping on the first backend.
    first = backends[0]
    sweep = {
        f"c={c}": model.simulate(options, parameters={"c": c}, backend=first)
        for c in (0.1, 1.0, 5.0)
    }
    compare(sweep, "x", title=f"Damping sweep ({first})", save_to=OUT / "damping_sweep.png")

    # 5. Results can be saved, reloaded and turned into DataFrames.
    results[first].save(OUT / "msd_result.csv")
    reloaded = Result.load(OUT / "msd_result.csv")
    print("reloaded metadata:", reloaded.metadata)
    print(reloaded.to_dataframe().head(3))

    # 6. Export an FMU for distribution.
    for name in backends:
        try:
            fmu = model.export_fmu(OUT / f"MassSpringDamper_{name}.fmu", backend=name)
            print(f"FMU ({name}): {fmu} ({fmu.stat().st_size} bytes)")
        except Exception as exc:  # report and continue so one backend cannot hide the others
            print(f"FMU export failed on {name}: {exc}")

    print(f"Outputs written to {OUT}")


if __name__ == "__main__":
    main()
