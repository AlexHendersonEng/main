import shutil

import numpy as np
import pytest

from polaris.backends import Capability, get_backend
from polaris.backends.openmodelica import OpenModelicaBackend
from polaris.types import FmuKind, ModelSource, SimulationOptions

MODEL = ModelSource(
    "Decay", files=(__import__("pathlib").Path(__file__).parent / "models" / "Decay.mo",)
)


def test_registered_and_capabilities():
    backend = get_backend("openmodelica")
    assert backend.supports(Capability.EXPORT_FMU | Capability.SIMULATE)


def test_rejects_bad_identifiers(tmp_path):
    backend = OpenModelicaBackend(executable="omc")
    bad = ModelSource('M"); system("x"); //')
    with pytest.raises(Exception, match="Invalid class name"):
        backend.simulate(bad, SimulationOptions(), tmp_path)


needs_omc = pytest.mark.skipif(shutil.which("omc") is None, reason="omc not installed")


@pytest.fixture(params=[True, False], ids=["ompython", "cli"])
def backend(request):
    return OpenModelicaBackend(use_ompython=request.param)


@pytest.mark.integration
@needs_omc
def test_simulate(backend, tmp_path):
    path = backend.simulate(MODEL, SimulationOptions(stop_time=1.0, step_size=0.1), tmp_path)
    lines = path.read_text().splitlines()
    assert lines[0].startswith('"time"')
    assert len(lines) >= 12
    last = dict(zip(lines[0].split(","), lines[-1].split(","), strict=True))
    assert float(last['"x"']) == pytest.approx(np.exp(-1), rel=1e-3)


@pytest.mark.integration
@needs_omc
def test_parameter_override(backend, tmp_path):
    src = ModelSource("Decay", MODEL.files, parameters={"k": 2.0})
    path = backend.simulate(src, SimulationOptions(stop_time=1.0), tmp_path)
    lines = path.read_text().splitlines()
    last = dict(zip(lines[0].split(","), lines[-1].split(","), strict=True))
    assert float(last['"x"']) == pytest.approx(np.exp(-2), rel=1e-3)


@pytest.mark.integration
@needs_omc
def test_export_fmu(backend, tmp_path):
    fmu = backend.export_fmu(MODEL, FmuKind.CO_SIMULATION, "2.0", tmp_path)
    assert fmu.suffix == ".fmu" and fmu.stat().st_size > 0


@pytest.mark.integration
@needs_omc
def test_failure_reports_log(backend, tmp_path):
    with pytest.raises(Exception, match="Nope"):
        backend.simulate(ModelSource("Nope"), SimulationOptions(), tmp_path)
