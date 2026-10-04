"""Integration test for the declarative co-simulation runner; optional deps are skipped."""

from __future__ import annotations

import shutil
from importlib.util import find_spec
from pathlib import Path

import pytest

from polaris import Model
from polaris.cosim import CoSimulation, FederateSpec, run_cosimulation

MODEL_FILE = Path(__file__).parent / "models" / "Decay.mo"
pytestmark = pytest.mark.skipif(find_spec("helics") is None, reason="install polaris[helics]")
needs_backend_and_compiler = pytest.mark.skipif(
    not (shutil.which("omc") or shutil.which("rumoca"))
    or not any(shutil.which(c) for c in ("clang", "gcc", "cc")),
    reason="test requires a Modelica compiler and C compiler",
)


@pytest.mark.integration
@needs_backend_and_compiler
def test_run_cosimulation_connects_producer_to_consumer(tmp_path):
    """A two-federate config should drive the consumer the same way a manual run does."""
    backend = "rumoca" if shutil.which("rumoca") else "openmodelica"
    model = Model("Decay", [MODEL_FILE])
    producer_fmu = model.export_fmu(tmp_path / "producer.fmu", backend=backend)
    consumer_fmu = model.export_fmu(tmp_path / "consumer.fmu", backend=backend)

    config = CoSimulation(
        federates=[
            FederateSpec(name="producer", fmu=producer_fmu, step_size=0.05),
            FederateSpec(name="consumer", fmu=consumer_fmu, step_size=0.05),
        ],
        connections=[{"source": "producer.y", "target": "consumer.u"}],
        stop_time=0.5,
    )
    results = run_cosimulation(config)

    assert set(results) == {"producer", "consumer"}
    producer_result = results["producer"]
    consumer_result = results["consumer"]
    assert producer_result.time == pytest.approx(consumer_result.time, abs=1e-12)
    # Driven by the producer's decaying output, the consumer should decay more slowly.
    assert consumer_result["x"][-1] > producer_result["x"][-1]


def test_run_cosimulation_requires_helics(monkeypatch):
    """A clear ImportError should surface instead of a bare ModuleNotFoundError."""
    import polaris.cosim.runner as runner_module

    def fake_import(name: str):
        raise ImportError(f"No module named '{name}'")

    monkeypatch.setattr(runner_module.importlib, "import_module", fake_import)
    config = CoSimulation(federates=[FederateSpec(name="a", fmu="a.fmu")], stop_time=1.0)
    with pytest.raises(ImportError, match="polaris\\[helics\\]"):
        runner_module.run_cosimulation(config)
