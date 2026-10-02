import shutil
from pathlib import Path

import numpy as np
import pytest

from polaris import FmuKind, Model, SimulationOptions
from polaris.fmu import FmuError, FmuNotRunnableError, inspect_fmu, validate_fmu
from polaris.fmu_runtime import run_fmu

MODEL_FILE = Path(__file__).parent / "models" / "Decay.mo"

needs_omc = pytest.mark.skipif(shutil.which("omc") is None, reason="omc not installed")
needs_rumoca = pytest.mark.skipif(shutil.which("rumoca") is None, reason="rumoca not installed")


@pytest.fixture(scope="module")
def om_fmu(tmp_path_factory):
    out = tmp_path_factory.mktemp("fmu") / "Decay.fmu"
    return Model("Decay", [MODEL_FILE]).export_fmu(out, backend="openmodelica")


@pytest.fixture(scope="module")
def rumoca_fmu(tmp_path_factory):
    out = tmp_path_factory.mktemp("fmu") / "Decay.fmu"
    return Model("Decay", [MODEL_FILE]).export_fmu(out, backend="rumoca")


def test_missing_and_corrupt_fmu(tmp_path):
    with pytest.raises(FmuError, match="not found"):
        inspect_fmu(tmp_path / "none.fmu")
    bad = tmp_path / "bad.fmu"
    bad.write_bytes(b"not a zip")
    with pytest.raises(FmuError, match="Cannot read"):
        inspect_fmu(bad)


@pytest.mark.integration
@needs_omc
def test_inspect_and_validate(om_fmu):
    info = inspect_fmu(om_fmu)
    assert info.model_name == "Decay" and info.fmi_version.startswith("2") and info.runnable
    assert info.parameters()["k"] == "1.0"
    report = validate_fmu(om_fmu)
    assert report.ok, report.errors


@pytest.mark.integration
@needs_omc
def test_run_fmu_with_parameters_and_substeps(om_fmu):
    opts = SimulationOptions(stop_time=1.0, step_size=0.1)
    coarse = run_fmu(om_fmu, opts, {"k": 2.0})
    fine = run_fmu(om_fmu, opts, {"k": 2.0}, substeps=100)
    assert len(coarse.time) == len(fine.time) == 11
    # Forward Euler improves as the communication step shrinks.
    exact = np.exp(-2)
    assert abs(fine["x"][-1] - exact) < abs(coarse["x"][-1] - exact)
    assert fine["x"][-1] == pytest.approx(exact, rel=1e-2)


@pytest.mark.integration
@needs_omc
def test_run_fmu_errors(om_fmu):
    with pytest.raises(KeyError, match="nope"):
        run_fmu(om_fmu, parameters={"nope": 1.0})
    with pytest.raises(ValueError, match="substeps"):
        run_fmu(om_fmu, substeps=0)


@pytest.mark.integration
@needs_omc
def test_output_filter(om_fmu):
    result = run_fmu(om_fmu, SimulationOptions(stop_time=1.0, outputs=("x",)))
    assert list(result) == ["x"]


@pytest.mark.integration
@needs_rumoca
def test_source_only_fmu_is_reported_not_run(rumoca_fmu):
    info = inspect_fmu(rumoca_fmu)
    assert not info.runnable and info.co_simulation and info.model_exchange
    with pytest.raises(FmuNotRunnableError, match="Source-code"):
        run_fmu(rumoca_fmu)
    report = validate_fmu(rumoca_fmu)
    assert report.ok and report.warnings


def test_fmu_kind_is_exported():
    assert FmuKind.CO_SIMULATION.value == "cs"
