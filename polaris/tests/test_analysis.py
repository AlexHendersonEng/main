import math
import shutil
from pathlib import Path

import numpy as np
import pytest

from polaris import Model, SimulationOptions
from polaris.analysis import global_sensitivity, jacobian, local_sensitivity
from polaris.backends import (
    Backend,
    Capability,
    UnsupportedCapabilityError,
    Version,
    register_backend,
    unregister_backend,
)

MODELS = Path(__file__).parent / "models"


class AnalyticBackend(Backend):
    """Pretends to simulate y = b * exp(-a t) (c ignored) without any compiler."""

    name = "analytic"
    capabilities = Capability.SIMULATE

    def is_available(self) -> bool:
        return True

    def version(self) -> Version:
        return Version(1)

    def simulate(self, source, options, work_dir):
        p = {"a": 1.0, "b": 2.0, "c": 1.0, **source.parameters}
        t = np.arange(0, options.stop_time + 1e-12, options.step_size or 0.1)
        path = work_dir / "r.csv"
        rows = ["time,y"] + [f"{ti},{p['b'] * math.exp(-p['a'] * ti)}" for ti in t]
        path.write_text("\n".join(rows) + "\n")
        return path


@pytest.fixture
def analytic():
    register_backend("analytic", AnalyticBackend)
    yield Model("TwoParam", backend="analytic")
    unregister_backend("analytic")


def test_local_sensitivity_matches_analytic(analytic):
    nominal = {"a": 1.0, "b": 2.0, "c": 1.0}
    opts = SimulationOptions(stop_time=1.0, step_size=0.1)
    s = local_sensitivity(analytic, nominal, ["y"], opts, workers=2)
    final = s.final()
    # y = b exp(-a t): dy/da = -b t exp(-a t), dy/db = exp(-a t), dy/dc = 0.
    assert final.loc["y", "a"] == pytest.approx(-2 * math.exp(-1), rel=1e-5)
    assert final.loc["y", "b"] == pytest.approx(math.exp(-1), rel=1e-5)
    assert final.loc["y", "c"] == pytest.approx(0.0, abs=1e-9)
    assert s.normalized_final().loc["y", "a"] == pytest.approx(-1.0, rel=1e-4)
    assert s.normalized_final().loc["y", "b"] == pytest.approx(1.0, rel=1e-6)
    assert s.gradients["a"]["y"].shape == s.time.shape


def test_forward_difference_and_validation(analytic):
    opts = SimulationOptions(stop_time=1.0, step_size=0.1)
    s = local_sensitivity(analytic, {"b": 2.0}, ["y"], opts, central=False, rel_step=1e-6)
    assert s.final().loc["y", "b"] == pytest.approx(math.exp(-1), rel=1e-4)
    with pytest.raises(ValueError):
        local_sensitivity(analytic, {}, ["y"])
    with pytest.raises(ValueError, match="workers"):
        local_sensitivity(analytic, {"b": 1.0}, ["y"], workers=0)


def test_zero_nominal_uses_absolute_step(analytic):
    # a=0 would make a relative step zero; the derivative must still be finite.
    s = local_sensitivity(analytic, {"a": 0.0}, ["y"], SimulationOptions(step_size=0.1))
    assert s.final().loc["y", "a"] == pytest.approx(-2.0, rel=1e-3)


def test_sobol_ranks_parameters(analytic):
    bounds = {"a": (0.5, 2.0), "b": (1.0, 3.0), "c": (0.0, 5.0)}
    g = global_sensitivity(
        analytic, bounds, "y", SimulationOptions(stop_time=1.0, step_size=0.5), n=128, seed=1
    )
    assert len(g.responses) == 128 * (3 + 2)
    idx = g.indices
    assert idx.loc["c", "ST"] == pytest.approx(0.0, abs=0.02)
    assert idx.loc["a", "ST"] > 0.05 and idx.loc["b", "ST"] > 0.05
    assert {"S1", "ST", "S1_conf", "ST_conf"} <= set(idx.columns)


def test_morris_and_errors(analytic):
    bounds = {"a": (0.5, 2.0), "c": (0.0, 5.0)}
    opts = SimulationOptions(stop_time=1.0, step_size=0.5)
    g = global_sensitivity(analytic, bounds, "y", opts, method="morris", n=10, seed=1)
    assert g.indices.loc["a", "mu_star"] > g.indices.loc["c", "mu_star"]
    assert g.indices.loc["c", "mu_star"] == pytest.approx(0.0, abs=1e-9)
    with pytest.raises(ValueError, match="Unknown method"):
        global_sensitivity(analytic, bounds, "y", method="nope")
    with pytest.raises(ValueError, match="empty"):
        global_sensitivity(analytic, {}, "y")


def test_custom_reducer(analytic):
    opts = SimulationOptions(stop_time=1.0, step_size=0.5)
    g = global_sensitivity(
        analytic,
        {"a": (0.5, 2.0), "b": (1, 3)},
        "y",
        opts,
        n=16,
        seed=0,
        reducer=lambda r: float(r["y"].max()),  # y is largest at t=0, where only b matters
    )
    assert g.indices.loc["a", "ST"] == pytest.approx(0.0, abs=0.05)


def test_jacobian_unsupported(analytic):
    with pytest.raises(UnsupportedCapabilityError, match="local_sensitivity"):
        jacobian(analytic)


needs_rumoca = pytest.mark.skipif(shutil.which("rumoca") is None, reason="rumoca not installed")
needs_omc = pytest.mark.skipif(shutil.which("omc") is None, reason="omc not installed")


@pytest.mark.integration
@needs_rumoca
def test_rumoca_native_jacobian():
    model = Model("TwoParam", [MODELS / "TwoParam.mo"], parameters={"a": 3.0})
    j = jacobian(model, at={"x": 0.5}, backend="rumoca")
    assert j.states == ("x",) and j.state_values == {"x": 0.5}
    assert j.state_matrix == pytest.approx(np.array([[-3.0]]))
    # der(x) = -a x, so d/da = -x = -0.5.
    assert j.parameter_matrix[0, j.parameters.index("a")] == pytest.approx(-0.5)


@pytest.mark.integration
@needs_rumoca
def test_jacobian_auto_selects_capable_backend():
    j = jacobian(Model("TwoParam", [MODELS / "TwoParam.mo"]))
    assert j.state_matrix[0, 0] == pytest.approx(-1.0)


@pytest.mark.integration
@needs_rumoca
@needs_omc
@pytest.mark.parametrize("backend", ["rumoca", "openmodelica"])
def test_real_backend_local_sensitivity(backend):
    model = Model("TwoParam", [MODELS / "TwoParam.mo"])
    opts = SimulationOptions(stop_time=1.0, step_size=0.1)
    s = local_sensitivity(model, {"a": 1.0, "b": 2.0}, ["y"], opts, backend=backend, workers=2)
    assert s.final().loc["y", "a"] == pytest.approx(-2 * math.exp(-1), rel=2e-2)
    assert s.final().loc["y", "b"] == pytest.approx(math.exp(-1), rel=2e-2)
