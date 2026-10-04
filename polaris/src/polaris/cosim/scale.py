"""Replicate FMU federates and measure co-simulation throughput."""

from __future__ import annotations

from collections.abc import Mapping, Sequence
from dataclasses import dataclass, replace
from time import perf_counter

from polaris.cosim.config import CoSimulation, FederateSpec
from polaris.cosim.runner import run_cosimulation


@dataclass(frozen=True)
class ScalingMeasurement:
    """Timing and throughput for one co-simulation benchmark run."""

    instances: int
    workers: int
    elapsed_seconds: float
    simulated_instance_seconds: float

    @property
    def throughput(self) -> float:
        """Simulated instance-seconds completed per wall-clock second."""
        return self.simulated_instance_seconds / self.elapsed_seconds


def replicate_federate(
    spec: FederateSpec,
    count: int,
    *,
    name_template: str = "{name}_{index}",
    parameters: Sequence[Mapping[str, float] | None] | None = None,
) -> tuple[FederateSpec, ...]:
    """Create ``count`` independently named copies of one FMU federate specification.

    Instance indices are zero-based. ``parameters`` optionally supplies one override
    mapping per instance; those values are merged over ``spec.parameters`` so common
    defaults can be specified once and selected FMU parameters varied by instance.

    Args:
        spec: Base FMU configuration to replicate.
        count: Number of instances; must be positive.
        name_template: Format string containing ``{name}`` and ``{index}``.
        parameters: Per-instance parameter overrides, one mapping per replica.

    Raises:
        ValueError: The count, parameter list, or generated names are invalid.
    """
    if count < 1:
        raise ValueError("count must be at least 1")
    if parameters is not None and len(parameters) != count:
        raise ValueError(f"Expected {count} parameter mappings, got {len(parameters)}")

    replicas: list[FederateSpec] = []
    for index in range(count):
        try:
            name = name_template.format(name=spec.name, index=index)
        except (KeyError, ValueError) as exc:
            raise ValueError("name_template must format using {name} and {index}") from exc
        if not name or "." in name:
            raise ValueError(f"Generated federate name {name!r} is invalid")
        overrides = parameters[index] if parameters is not None else None
        merged_parameters = dict(spec.parameters or {})
        merged_parameters.update(overrides or {})
        replicas.append(
            replace(
                spec,
                name=name,
                parameters=merged_parameters or None,
            )
        )
    names = [replica.name for replica in replicas]
    if len(set(names)) != len(names):
        raise ValueError(f"name_template generated duplicate federate names: {names}")
    return tuple(replicas)


def benchmark_cosimulation(
    config: CoSimulation,
    *,
    worker_counts: Sequence[int] = (1, 2),
) -> list[ScalingMeasurement]:
    """Run a federation with several process counts and report elapsed time/throughput.

    Each measurement runs the full federation afresh. Results are intentionally not
    retained; use :func:`polaris.cosim.runner.run_cosimulation` for production runs.
    """
    if not worker_counts:
        raise ValueError("worker_counts must not be empty")
    measurements: list[ScalingMeasurement] = []
    for workers in worker_counts:
        if workers < 1:
            raise ValueError("worker counts must be positive")
        start = perf_counter()
        results = run_cosimulation(config, workers=workers)
        elapsed = perf_counter() - start
        simulated_instance_seconds = sum(
            config.stop_time - spec.start_time for spec in config.federates
        )
        measurements.append(
            ScalingMeasurement(
                instances=len(results),
                workers=workers,
                elapsed_seconds=elapsed,
                simulated_instance_seconds=simulated_instance_seconds,
            )
        )
    return measurements
