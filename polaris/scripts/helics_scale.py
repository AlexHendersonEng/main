"""Run a replicated chain of Modelica FMUs through the declarative HELICS runner.

Examples:

    uv run --extra helics python scripts/helics_scale.py --instances 4 --workers 2
    uv run --extra helics python scripts/helics_scale.py --instances 4 --benchmark
    uv run polaris cosim run scripts/output/helics_scale/cosim.toml --workers 2 \
        --output-dir scripts/output/helics_scale/results

Each instance publishes ``y`` to the next instance's ``u`` input. The script exports
one FMU, replicates its specification, saves results and an overlay plot, and can
optionally benchmark process counts.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from polaris import Model
from polaris.backends import available_backends
from polaris.cosim import (
    Connection,
    CoSimulation,
    FederateSpec,
    benchmark_cosimulation,
    replicate_federate,
    run_cosimulation,
)
from polaris.plotting import compare

HERE = Path(__file__).parent
OUTPUT = HERE / "output" / "helics_scale"


def main() -> None:
    """Export one FMU, replicate a connected chain, and save simulation results."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--instances", type=int, default=4, help="Number of FMU instances (>=2)")
    parser.add_argument("--workers", type=int, default=1, help="Process workers for the run")
    parser.add_argument("--stop", type=float, default=2.0, help="Simulation stop time")
    parser.add_argument("--step", type=float, default=0.05, help="Communication step size")
    parser.add_argument("--backend", choices=available_backends(), default=None)
    parser.add_argument(
        "--benchmark",
        action="store_true",
        help="Also benchmark one and --workers processes (duplicate count is removed)",
    )
    args = parser.parse_args()
    if args.instances < 2:
        parser.error("--instances must be at least 2 for a connected chain")
    if args.workers < 1:
        parser.error("--workers must be at least 1")

    OUTPUT.mkdir(parents=True, exist_ok=True)
    backend = args.backend or ("rumoca" if "rumoca" in available_backends() else None)
    model = Model("Decay", (HERE.parent / "tests" / "models" / "Decay.mo",))
    fmu_path = model.export_fmu(OUTPUT / "decay.fmu", backend=backend)
    instances = replicate_federate(
        FederateSpec(name="plant", fmu=fmu_path, step_size=args.step),
        args.instances,
        parameters=[{"k": 0.8 + 0.1 * index} for index in range(args.instances)],
    )
    connections = [
        Connection(source=f"{source.name}.y", target=f"{target.name}.u")
        for source, target in zip(instances, instances[1:], strict=False)
    ]
    config = CoSimulation(
        federates=instances,
        connections=connections,
        stop_time=args.stop,
        broker_name="polaris-helics-scale-example",
    )

    # Save a TOML config so the same federation can also be run through the CLI.
    toml_path = OUTPUT / "cosim.toml"
    toml_path.write_text(
        "\n".join(
            [
                f"stop_time = {args.stop!r}",
                "",
                *[
                    line
                    for spec in instances
                    for line in (
                        "[[federates]]",
                        f"name = {json.dumps(spec.name)}",
                        f"fmu = {json.dumps(str(spec.fmu))}",
                        f"step_size = {spec.step_size!r}",
                        f"parameters = {{ k = {(spec.parameters or {}).get('k', 1.0)!r} }}",
                        "",
                    )
                ],
                *[
                    line
                    for connection in connections
                    for line in (
                        "[[connections]]",
                        f"source = {json.dumps(connection.source)}",
                        f"target = {json.dumps(connection.target)}",
                        "",
                    )
                ],
            ]
        ),
        encoding="utf-8",
    )
    # Reload the emitted configuration to verify the CLI input file is usable.
    CoSimulation.from_toml(toml_path)

    results = run_cosimulation(config, workers=args.workers)
    for index, (name, result) in enumerate(results.items()):
        result.save(OUTPUT / f"federate-{index:03d}.csv")
        print(f"{name}: final x={result['x'][-1]:.6f}")
    compare(
        results,
        "x",
        title=f"HELICS replicated chain ({args.instances} FMUs)",
        save_to=OUTPUT / "replicated_chain.png",
    )

    if args.benchmark:
        worker_counts = tuple(dict.fromkeys((1, args.workers)))
        for measurement in benchmark_cosimulation(config, worker_counts=worker_counts):
            print(
                f"{measurement.workers} worker(s): {measurement.elapsed_seconds:.3f}s, "
                f"{measurement.throughput:.2f} simulated instance-s/s"
            )
    print(f"Config, CSV results, FMU, and plot written to {OUTPUT}")


if __name__ == "__main__":
    main()
