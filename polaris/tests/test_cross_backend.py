"""Integration tests that compare the same Modelica model across compiler backends."""

from __future__ import annotations

import shutil
from pathlib import Path

import numpy as np
import pytest

from polaris import Model, SimulationOptions
from polaris.fmu import inspect_fmu
from polaris.fmu_runtime import run_fmu

MODEL_FILE = Path(__file__).parent / "models" / "Decay.mo"
needs_both_backends = pytest.mark.skipif(
    shutil.which("omc") is None or shutil.which("rumoca") is None,
    reason="cross-backend tests need both omc and rumoca",
)
needs_compiler = pytest.mark.skipif(
    not any(shutil.which(name) for name in ("clang", "gcc", "cc")),
    reason="Rumoca source FMU requires a C compiler",
)


def _model() -> Model:
    """Return the shared analytic decay model with a non-default decay rate."""
    return Model("Decay", [MODEL_FILE], parameters={"k": 2.0})


def _options() -> SimulationOptions:
    """Use an identical time grid and variable selection for both compilers."""
    return SimulationOptions(stop_time=1.0, step_size=0.05, outputs=("x", "y"))


@pytest.mark.integration
@needs_both_backends
def test_direct_simulation_agrees_across_backends():
    model = _model()
    options = _options()
    openmodelica = model.simulate(options, backend="openmodelica")
    rumoca = model.simulate(options, backend="rumoca")

    # The input model has y = 2*x and x' = -2*x with x(0)=1; this checks values
    # against a known solution, not merely that the two implementations agree with each other.
    expected_x = np.exp(-2.0 * openmodelica.time)
    assert openmodelica.time == pytest.approx(rumoca.time, abs=1e-12)
    assert list(openmodelica) == list(rumoca) == ["x", "y"]
    assert openmodelica["x"] == pytest.approx(expected_x, rel=1e-3, abs=1e-6)
    assert rumoca["x"] == pytest.approx(expected_x, rel=1e-3, abs=1e-6)
    assert openmodelica["x"] == pytest.approx(rumoca["x"], rel=1e-3, abs=1e-6)
    assert openmodelica["y"] == pytest.approx(rumoca["y"], rel=1e-3, abs=1e-6)
    assert openmodelica.metadata["backend"] == "openmodelica"
    assert rumoca.metadata["backend"] == "rumoca"


@pytest.mark.integration
@needs_both_backends
@needs_compiler
def test_fmu_simulations_agree_with_direct_and_analytic_results(tmp_path):
    model = _model()
    options = _options()
    direct = {name: model.simulate(options, backend=name) for name in ("openmodelica", "rumoca")}
    fmus = {
        name: model.export_fmu(tmp_path / f"{name}.fmu", backend=name)
        for name in ("openmodelica", "rumoca")
    }
    info = {name: inspect_fmu(path) for name, path in fmus.items()}

    assert info["openmodelica"].runnable
    assert not info["rumoca"].runnable and "c-code" in info["rumoca"].platforms
    # Exported FMUs retain compiler defaults; pass the same override used by direct runs.
    results = {name: run_fmu(path, options, {"k": 2.0}, substeps=20) for name, path in fmus.items()}

    for name, result in results.items():
        assert result.time == pytest.approx(direct[name].time, abs=1e-12)
        assert list(result) == ["x", "y"]
        assert result["x"] == pytest.approx(np.exp(-2.0 * result.time), rel=2e-2, abs=1e-5)
        assert result["x"] == pytest.approx(direct[name]["x"], rel=2e-2, abs=1e-5)

    # Distinct compilers/solvers and FMU integrators need not be bit-identical, but
    # the shared 20-substep communication grid should keep their trajectories close.
    assert results["openmodelica"]["x"] == pytest.approx(results["rumoca"]["x"], rel=2e-2, abs=1e-5)
