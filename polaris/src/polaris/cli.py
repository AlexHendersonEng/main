"""Command-line interface for building, simulating and analysing Modelica models."""

from __future__ import annotations

import argparse
import json
import sys
from collections.abc import Sequence
from math import isfinite
from pathlib import Path

from polaris.analysis import jacobian
from polaris.backends import (
    BackendError,
    available_backends,
    get_backend,
    registered_backends,
)
from polaris.cosim import CoSimulation, benchmark_cosimulation, run_cosimulation
from polaris.fmu import FmuError, inspect_fmu, validate_fmu
from polaris.fmu_runtime import run_fmu
from polaris.interactive import InteractiveSession, run_dashboard
from polaris.model import Model
from polaris.types import FmuKind, SimulationOptions


def _add_model_arguments(parser: argparse.ArgumentParser) -> None:
    """Add Modelica source and backend arguments shared by model commands."""
    parser.add_argument("model", help="Fully qualified Modelica class name")
    parser.add_argument(
        "--file", action="append", type=Path, required=True, help="Modelica .mo file"
    )
    parser.add_argument("--library", action="append", default=[], help="Modelica library to load")
    parser.add_argument("--backend", help="Backend name (default: first available backend)")
    parser.add_argument(
        "--parameter",
        action="append",
        default=[],
        metavar="NAME=VALUE",
        help="Model parameter override; may be repeated",
    )


def _add_simulation_options(parser: argparse.ArgumentParser) -> None:
    """Add common fixed-grid simulation options."""
    parser.add_argument("--start", type=float, default=0.0, help="Start time (default: 0)")
    parser.add_argument("--stop", type=float, default=1.0, help="Stop time (default: 1)")
    parser.add_argument("--step", type=float, help="Output step size (backend default if omitted)")
    parser.add_argument("--tolerance", type=float, default=1e-6)
    parser.add_argument("--solver", help="Backend-specific solver name")
    parser.add_argument(
        "--variable", action="append", default=[], help="Variable to record (repeatable)"
    )


def _parser() -> argparse.ArgumentParser:
    """Build the parser without performing backend discovery until requested."""
    parser = argparse.ArgumentParser(
        prog="polaris",
        description="Build, simulate, analyse and export Modelica models.",
    )
    commands = parser.add_subparsers(dest="command", required=True)

    simulate = commands.add_parser("simulate", help="simulate a Modelica model to CSV")
    _add_model_arguments(simulate)
    _add_simulation_options(simulate)
    simulate.add_argument("--output", type=Path, required=True, help="Destination result CSV")

    fmu = commands.add_parser("fmu", help="export, inspect, validate or run an FMU")
    fmu_commands = fmu.add_subparsers(dest="fmu_command", required=True)
    export = fmu_commands.add_parser("export", help="export a Modelica model to FMU")
    _add_model_arguments(export)
    export.add_argument("destination", type=Path, help="Destination .fmu file")
    export.add_argument(
        "--kind", choices=[kind.value for kind in FmuKind], default=FmuKind.CO_SIMULATION.value
    )
    export.add_argument("--fmi-version", default="2.0")

    for action in ("inspect", "validate"):
        sub = fmu_commands.add_parser(action, help=f"{action} an FMU")
        sub.add_argument("fmu", type=Path)
        if action == "validate":
            sub.add_argument("--no-smoke-test", action="store_true")

    run = fmu_commands.add_parser("run", help="run an FMU")
    run.add_argument("fmu", type=Path)
    run.add_argument("--output", type=Path, required=True, help="Destination result CSV")
    _add_simulation_options(run)
    run.add_argument(
        "--parameter",
        action="append",
        default=[],
        metavar="NAME=VALUE",
        help="FMU start value override; may be repeated",
    )
    run.add_argument("--substeps", type=int, default=1)
    run.add_argument("--no-compile-sources", action="store_true")

    jac = commands.add_parser("jacobian", help="compute native state and parameter Jacobians")
    _add_model_arguments(jac)
    jac.add_argument("--at", action="append", default=[], metavar="STATE=VALUE")

    sensitivity = commands.add_parser("sensitivity", help="analyse model parameter sensitivity")
    modes = sensitivity.add_subparsers(dest="sensitivity_command", required=True)
    local = modes.add_parser("local", help="finite-difference local sensitivity")
    _add_model_arguments(local)
    _add_simulation_options(local)
    local.add_argument("--output-file", type=Path, help="Optional CSV file for final sensitivities")
    local.add_argument("--rel-step", type=float, default=1e-3)
    local.add_argument(
        "--forward", action="store_true", help="Use forward instead of central differences"
    )
    local.add_argument("--workers", type=int, default=1)

    global_analysis = modes.add_parser("global", help="Sobol or Morris global sensitivity")
    _add_model_arguments(global_analysis)
    _add_simulation_options(global_analysis)
    global_analysis.add_argument(
        "--bound", action="append", default=[], metavar="NAME=MIN,MAX", required=True
    )
    global_analysis.add_argument("--method", choices=("sobol", "morris"), default="sobol")
    global_analysis.add_argument("--samples", type=int, default=64)
    global_analysis.add_argument("--seed", type=int)
    global_analysis.add_argument("--workers", type=int, default=1)
    global_analysis.add_argument("--output-variable", required=True)
    global_analysis.add_argument("--output-file", type=Path, help="Optional CSV file for indices")

    dashboard = commands.add_parser(
        "dashboard", help="serve a browser dashboard for a Co-Simulation FMU"
    )
    dashboard.add_argument("fmu", type=Path)
    dashboard.add_argument(
        "--control", action="append", default=[], metavar="NAME=MIN,MAX", required=True
    )
    dashboard.add_argument("--variable", action="append", default=[])
    dashboard.add_argument("--step-size", type=float, default=0.01)
    dashboard.add_argument("--step-duration", type=float)
    dashboard.add_argument("--host", default="127.0.0.1")
    dashboard.add_argument("--port", type=int, default=8050)
    dashboard.add_argument("--debug", action="store_true")
    dashboard.add_argument("--force-fixed-parameters", action="store_true")

    commands.add_parser("backends", help="list registered and available compiler backends")

    cosim = commands.add_parser("cosim", help="run or benchmark a HELICS co-simulation")
    cosim_commands = cosim.add_subparsers(dest="cosim_command", required=True)
    cosim_run = cosim_commands.add_parser("run", help="run a TOML-defined co-simulation")
    cosim_run.add_argument("config", type=Path, help="Co-simulation TOML configuration")
    cosim_run.add_argument("--workers", type=int, default=1, help="Worker processes (default: 1)")
    cosim_run.add_argument(
        "--output-dir", type=Path, required=True, help="Directory for per-federate result CSVs"
    )

    cosim_benchmark = cosim_commands.add_parser(
        "benchmark", help="benchmark a TOML-defined co-simulation"
    )
    cosim_benchmark.add_argument("config", type=Path, help="Co-simulation TOML configuration")
    cosim_benchmark.add_argument(
        "--workers",
        type=int,
        action="append",
        default=None,
        help="Worker count to benchmark; may be repeated (default: 1 and 2)",
    )
    return parser


def _mapping(items: Sequence[str]) -> dict[str, float]:
    """Parse repeated NAME=VALUE arguments with helpful errors."""
    parsed: dict[str, float] = {}
    for item in items:
        name, separator, value = item.partition("=")
        if not separator or not name or not value:
            raise ValueError(f"Expected NAME=VALUE, got {item!r}")
        if name in parsed:
            raise ValueError(f"'{name}' was specified more than once")
        try:
            parsed[name] = float(value)
        except ValueError as exc:
            raise ValueError(f"Invalid numeric value in {item!r}") from exc
        if not isfinite(parsed[name]):
            raise ValueError(f"Expected a finite numeric value in {item!r}")
    return parsed


def _bounds(items: Sequence[str]) -> dict[str, tuple[float, float]]:
    """Parse repeated NAME=MIN,MAX arguments and require increasing finite bounds."""
    parsed: dict[str, tuple[float, float]] = {}
    for item in items:
        name, separator, value = item.partition("=")
        if not separator or not name:
            raise ValueError(f"Expected NAME=MIN,MAX, got {item!r}")
        if name in parsed:
            raise ValueError(f"'{name}' was specified more than once")
        try:
            values = [float(part) for part in value.split(",")]
        except ValueError as exc:
            raise ValueError(f"Invalid numeric bounds in {item!r}") from exc
        if len(values) != 2 or not all(isfinite(part) for part in values):
            raise ValueError(f"Expected two finite bounds in {item!r}")
        lower, upper = values
        if lower >= upper:
            raise ValueError(f"Bounds must be increasing in {item!r}")
        parsed[name] = (lower, upper)
    return parsed


def _model(args: argparse.Namespace) -> Model:
    """Create a Model from parsed common CLI arguments."""
    return Model(
        args.model,
        tuple(args.file),
        tuple(args.library),
        _mapping(args.parameter),
        args.backend,
    )


def _options(args: argparse.Namespace) -> SimulationOptions:
    """Create solver options from a parsed command."""
    if args.stop <= args.start:
        raise ValueError("--stop must be greater than --start")
    if args.step is not None and args.step <= 0:
        raise ValueError("--step must be positive")
    if args.tolerance <= 0:
        raise ValueError("--tolerance must be positive")
    return SimulationOptions(
        start_time=args.start,
        stop_time=args.stop,
        step_size=args.step,
        tolerance=args.tolerance,
        solver=args.solver,
        outputs=tuple(args.variable),
    )


def _dispatch(args: argparse.Namespace) -> int:
    """Execute a validated command and print a concise result."""
    if args.command == "backends":
        available = set(available_backends())
        for name in registered_backends():
            backend = get_backend(name)
            if name in available:
                print(
                    f"{name}: available; version {backend.version()}; "
                    f"capabilities {backend.capabilities}"
                )
            else:
                print(f"{name}: unavailable; capabilities {backend.capabilities}")
        return 0

    if args.command == "simulate":
        result = _model(args).simulate(
            _options(args), parameters=_mapping(args.parameter), backend=args.backend
        )
        args.output.parent.mkdir(parents=True, exist_ok=True)
        result.save(args.output)
        print(f"Saved {len(result.time)} points and {len(result)} variables to {args.output}")
        return 0

    if args.command == "fmu":
        if args.fmu_command == "export":
            path = _model(args).export_fmu(
                args.destination,
                kind=FmuKind(args.kind),
                fmi_version=args.fmi_version,
                backend=args.backend,
            )
            print(path)
        elif args.fmu_command == "inspect":
            info = inspect_fmu(args.fmu)
            print(
                json.dumps(
                    {
                        "path": str(info.path),
                        "model": info.model_name,
                        "fmi_version": info.fmi_version,
                        "co_simulation": info.co_simulation,
                        "model_exchange": info.model_exchange,
                        "platforms": info.platforms,
                        "runnable": info.runnable,
                        "parameters": info.parameters(),
                    },
                    indent=2,
                )
            )
        elif args.fmu_command == "validate":
            report = validate_fmu(args.fmu, smoke_test=not args.no_smoke_test)
            print(
                json.dumps(
                    {
                        "ok": report.ok,
                        "errors": report.errors,
                        "warnings": report.warnings,
                    },
                    indent=2,
                )
            )
            return 0 if report.ok else 1
        else:
            result = run_fmu(
                args.fmu,
                _options(args),
                _mapping(args.parameter),
                substeps=args.substeps,
                compile_sources=not args.no_compile_sources,
            )
            args.output.parent.mkdir(parents=True, exist_ok=True)
            result.save(args.output)
            print(f"Saved {len(result.time)} points and {len(result)} variables to {args.output}")
        return 0

    if args.command == "jacobian":
        values = _mapping(args.at)
        jac = jacobian(_model(args), values, backend=args.backend)
        print(f"Evaluation time: {jac.time:g}")
        print(f"States: {', '.join(jac.states)}")
        print("State matrix (d der(states) / d states):")
        print(jac.state_matrix)
        print(f"Parameters: {', '.join(jac.parameters)}")
        print("Parameter matrix (d der(states) / d parameters):")
        print(jac.parameter_matrix)
        return 0

    if args.command == "sensitivity":
        model = _model(args)
        options = _options(args)
        if args.sensitivity_command == "local":
            from polaris.analysis import local_sensitivity

            params = _mapping(args.parameter)
            table = local_sensitivity(
                model,
                params,
                args.variable,
                options,
                rel_step=args.rel_step,
                central=not args.forward,
                backend=args.backend,
                workers=args.workers,
            ).final()
            print(table.to_string())
        else:
            from polaris.analysis import global_sensitivity

            table = global_sensitivity(
                model,
                _bounds(args.bound),
                args.output_variable,
                options,
                method=args.method,
                n=args.samples,
                backend=args.backend,
                workers=args.workers,
                seed=args.seed,
            ).indices
            print(table.to_string())
        if args.output_file:
            args.output_file.parent.mkdir(parents=True, exist_ok=True)
            table.to_csv(args.output_file)
            print(f"Saved sensitivity table to {args.output_file}")
        return 0

    if args.command == "dashboard":
        controls = _bounds(args.control)
        with InteractiveSession(
            args.fmu,
            step_size=args.step_size,
            record=args.variable or None,
        ) as session:
            run_dashboard(
                session,
                controls,
                variables=args.variable or None,
                step_duration=args.step_duration,
                force_fixed_parameters=args.force_fixed_parameters,
                host=args.host,
                port=args.port,
                debug=args.debug,
            )
        return 0

    if args.command == "cosim":
        config = CoSimulation.from_toml(args.config)
        if args.cosim_command == "run":
            if args.workers < 1:
                raise ValueError("--workers must be at least 1")
            results = run_cosimulation(config, workers=args.workers)
            args.output_dir.mkdir(parents=True, exist_ok=True)
            for index, (name, result) in enumerate(results.items()):
                path = args.output_dir / f"federate-{index:03d}.csv"
                result.save(path)
                print(f"{name}: saved {len(result.time)} points to {path}")
            return 0

        worker_counts = args.workers if args.workers is not None else (1, 2)
        measurements = benchmark_cosimulation(config, worker_counts=worker_counts)
        print("workers  federates  elapsed (s)  throughput (instance-s/s)")
        for measurement in measurements:
            print(
                f"{measurement.workers:7d}  {measurement.instances:9d}  "
                f"{measurement.elapsed_seconds:11.3f}  {measurement.throughput:25.3f}"
            )
        return 0

    raise AssertionError(f"Unhandled command: {args.command}")


def main(argv: Sequence[str] | None = None) -> int:
    """Run the Polaris CLI and return a process exit status.

    Args:
        argv: Arguments excluding the program name. Defaults to ``sys.argv[1:]``.
    """
    parser = _parser()
    args = parser.parse_args(argv)
    try:
        return _dispatch(args)
    except (
        BackendError,
        FmuError,
        ImportError,
        KeyError,
        OSError,
        RuntimeError,
        ValueError,
    ) as exc:
        print(f"polaris: error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
