import shutil
from pathlib import Path

import numpy as np
import pytest

from polaris.backends import Capability, get_backend
from polaris.backends.base import BackendError
from polaris.backends.rumoca import RumocaBackend
from polaris.types import FmuKind, ModelSource, SimulationOptions

MODEL = ModelSource("Decay", files=(Path(__file__).parent / "models" / "Decay.mo",))

needs_rumoca = pytest.mark.skipif(shutil.which("rumoca") is None, reason="rumoca not installed")


def _last_row(path: Path) -> dict[str, float]:
    lines = path.read_text().splitlines()
    return dict(zip(lines[0].split(","), map(float, lines[-1].split(",")), strict=True))


def test_registered_and_capabilities():
    backend = get_backend("rumoca")
    assert backend.supports(Capability.SIMULATE | Capability.EXPORT_FMU)


def test_rejects_bad_identifiers(tmp_path):
    backend = RumocaBackend(executable="rumoca")
    with pytest.raises(BackendError, match="Invalid class name"):
        backend.simulate(ModelSource('M"; x', MODEL.files), SimulationOptions(), tmp_path)
    bad_param = ModelSource("Decay", MODEL.files, parameters={"k) //": 1.0})
    with pytest.raises(BackendError, match="Invalid parameter"):
        backend.simulate(bad_param, SimulationOptions(), tmp_path)


def test_requires_files_and_zero_start(tmp_path):
    backend = RumocaBackend(executable="rumoca")
    with pytest.raises(BackendError, match="at least one"):
        backend.simulate(ModelSource("Decay"), SimulationOptions(), tmp_path)
    with pytest.raises(BackendError, match="start_time"):
        backend.simulate(MODEL, SimulationOptions(start_time=1.0), tmp_path)


def test_unsupported_fmi_version(tmp_path):
    backend = RumocaBackend(executable="rumoca")
    with pytest.raises(BackendError, match="FMI 1.0"):
        backend.export_fmu(MODEL, FmuKind.CO_SIMULATION, "1.0", tmp_path)


@pytest.mark.integration
@needs_rumoca
def test_version():
    assert RumocaBackend().version().major == 0


@pytest.mark.integration
@needs_rumoca
def test_simulate(tmp_path):
    path = RumocaBackend().simulate(
        MODEL, SimulationOptions(stop_time=1.0, step_size=0.1), tmp_path
    )
    assert path.read_text().splitlines()[0].startswith("time,")
    assert _last_row(path)["x"] == pytest.approx(np.exp(-1), rel=1e-3)


@pytest.mark.integration
@needs_rumoca
def test_parameter_override_and_output_filter(tmp_path):
    src = ModelSource("Decay", MODEL.files, parameters={"k": 2.0})
    opts = SimulationOptions(stop_time=1.0, outputs=("x",))
    path = RumocaBackend().simulate(src, opts, tmp_path)
    assert path.read_text().splitlines()[0] == "time,x"
    assert _last_row(path)["x"] == pytest.approx(np.exp(-2), rel=1e-3)


@pytest.mark.integration
@needs_rumoca
@pytest.mark.parametrize("fmi", ["2.0", "3.0"])
def test_export_fmu(tmp_path, fmi):
    fmu = RumocaBackend().export_fmu(MODEL, FmuKind.CO_SIMULATION, fmi, tmp_path)
    assert fmu.name == "Decay.fmu" and fmu.stat().st_size > 0


@pytest.mark.integration
@needs_rumoca
def test_failure_reports_output(tmp_path):
    with pytest.raises(BackendError, match="simulation failed"):
        RumocaBackend().simulate(
            ModelSource("Nope", (tmp_path / "missing.mo",)), SimulationOptions(), tmp_path
        )
