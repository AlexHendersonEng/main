"""Tests for FMU replication and co-simulation throughput measurements."""

from __future__ import annotations

import pytest

from polaris.cosim import (
    CoSimulation,
    FederateSpec,
    ScalingMeasurement,
    benchmark_cosimulation,
    replicate_federate,
)


def test_replicate_federate_merges_instance_parameters():
    base = FederateSpec(
        name="plant",
        fmu="plant.fmu",
        parameters={"k": 1.0, "c": 0.1},
    )
    replicas = replicate_federate(
        base,
        3,
        parameters=[{"k": 1.0}, {"k": 2.0}, {"k": 3.0}],
    )

    assert [spec.name for spec in replicas] == ["plant_0", "plant_1", "plant_2"]
    assert [spec.parameters for spec in replicas] == [
        {"k": 1.0, "c": 0.1},
        {"k": 2.0, "c": 0.1},
        {"k": 3.0, "c": 0.1},
    ]
    assert all(spec.fmu == base.fmu and spec.step_size == base.step_size for spec in replicas)


def test_replicate_federate_validates_count_parameters_and_names():
    base = FederateSpec(name="plant", fmu="plant.fmu")
    with pytest.raises(ValueError, match="at least 1"):
        replicate_federate(base, 0)

    with pytest.raises(ValueError, match="Expected 2"):
        replicate_federate(base, 2, parameters=[{}])

    with pytest.raises(ValueError, match="duplicate"):
        replicate_federate(base, 2, name_template="same")


def test_benchmark_reports_instance_seconds(monkeypatch):
    import polaris.cosim.scale as scale_module

    def fake_run(config: CoSimulation, *, workers: int):
        return {spec.name: object() for spec in config.federates}

    monkeypatch.setattr(scale_module, "run_cosimulation", fake_run)
    config = CoSimulation(
        federates=[
            FederateSpec(name="a", fmu="a.fmu", start_time=0.0),
            FederateSpec(name="b", fmu="b.fmu", start_time=1.0),
        ],
        stop_time=3.0,
    )

    measurements = benchmark_cosimulation(config, worker_counts=(1, 2))

    assert [measurement.workers for measurement in measurements] == [1, 2]
    assert all(isinstance(measurement, ScalingMeasurement) for measurement in measurements)
    assert all(measurement.instances == 2 for measurement in measurements)
    assert all(measurement.simulated_instance_seconds == 5.0 for measurement in measurements)
    assert all(measurement.throughput > 0 for measurement in measurements)
