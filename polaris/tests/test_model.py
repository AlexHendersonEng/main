from pathlib import Path

import numpy as np
import pytest

from polaris import Model, Result, SimulationOptions
from polaris.backends import Backend, Capability, Version, register_backend, unregister_backend
from polaris.model import resolve_backend

MODEL_FILE = Path(__file__).parent / "models" / "Decay.mo"


class FakeBackend(Backend):
    """Writes a fixed CSV and records what it was asked to do."""

    name = "fake_sim"
    capabilities = Capability.SIMULATE | Capability.EXPORT_FMU
    calls: list = []

    def is_available(self) -> bool:
        return True

    def version(self) -> Version:
        return Version(1)

    def simulate(self, source, options, work_dir):
        FakeBackend.calls.append((source, options))
        path = work_dir / "out.csv"
        path.write_text('"time","x"\n0,1\n1,2\n')
        return path

    def export_fmu(self, source, kind, fmi_version, work_dir):
        path = work_dir / "m.fmu"
        path.write_bytes(b"fmu")
        return path


@pytest.fixture
def fake():
    FakeBackend.calls = []
    register_backend("fake_sim", FakeBackend)
    yield
    unregister_backend("fake_sim")


def test_result_mapping_and_roundtrip(tmp_path):
    r = Result(np.array([0.0, 1.0]), {"x": np.array([1.0, 2.0])}, {"backend": "b"})
    assert r["x"][1] == 2.0 and r["time"][1] == 1.0 and list(r) == ["x"] and len(r) == 1
    assert r.final() == {"x": 2.0}
    assert list(r.to_dataframe().columns) == ["x"]
    r.save(tmp_path / "r.csv")
    loaded = Result.load(tmp_path / "r.csv")
    assert np.allclose(loaded["x"], r["x"]) and loaded.metadata == {"backend": "b"}
    with pytest.raises(KeyError, match="nope"):
        r["nope"]


def test_result_rejects_bad_csv(tmp_path):
    (tmp_path / "a.csv").write_text("a,b\n1,2\n")
    with pytest.raises(ValueError, match="first column"):
        Result.from_csv(tmp_path / "a.csv")
    (tmp_path / "b.csv").write_text("time,x\n")
    with pytest.raises(ValueError, match="no data"):
        Result.from_csv(tmp_path / "b.csv")


def test_simulate_merges_parameters_and_loads_result(fake):
    model = Model("M", [MODEL_FILE], parameters={"k": 1.0, "a": 5.0}, backend="fake_sim")
    result = model.simulate(SimulationOptions(stop_time=2.0), parameters={"k": 3.0})
    source, options = FakeBackend.calls[0]
    assert source.parameters == {"k": 3.0, "a": 5.0} and options.stop_time == 2.0
    assert model.parameters["k"] == 1.0
    assert result.metadata["backend"] == "fake_sim" and result["x"][-1] == 2.0


def test_export_fmu(fake, tmp_path):
    model = Model("M", backend="fake_sim")
    assert model.export_fmu(tmp_path).read_bytes() == b"fmu"
    assert model.export_fmu(tmp_path / "sub" / "x.fmu").name == "x.fmu"


def test_with_parameters_and_backend_resolution(fake):
    assert Model("M").with_parameters(k=2.0).parameters == {"k": 2.0}
    assert resolve_backend("fake_sim").name == "fake_sim"
    with pytest.raises(KeyError):
        resolve_backend("missing")


@pytest.mark.integration
@pytest.mark.parametrize("backend", ["openmodelica", "rumoca"])
def test_real_backends_agree(backend):
    model = Model("Decay", [MODEL_FILE], parameters={"k": 2.0})
    result = model.simulate(SimulationOptions(stop_time=1.0), backend=backend)
    assert result["x"][-1] == pytest.approx(np.exp(-2), rel=1e-3)
    assert result.metadata["backend"] == backend


def test_from_csv_drops_only_exact_duplicate_rows(tmp_path):
    path = tmp_path / "r.csv"
    # Row 3 repeats row 2 exactly (dropped); rows 4-5 share a time but differ (an event).
    path.write_text("time,x\n0,1\n1,2\n1,2\n2,3\n2,0\n")
    result = Result.from_csv(path)
    assert result.time.tolist() == [0, 1, 2, 2]
    assert result["x"].tolist() == [1, 2, 3, 0]
