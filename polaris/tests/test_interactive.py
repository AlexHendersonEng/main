import shutil
from pathlib import Path

import matplotlib
import numpy as np
import pytest

from polaris import Model
from polaris.fmu import FmuError
from polaris.interactive import InteractiveSession, LivePlot, SessionError, run_live

MODEL_FILE = Path(__file__).parent / "models" / "Driven.mo"
needs_omc = pytest.mark.skipif(shutil.which("omc") is None, reason="omc not installed")
needs_rumoca = pytest.mark.skipif(shutil.which("rumoca") is None, reason="rumoca not installed")
needs_compiler = pytest.mark.skipif(
    not any(shutil.which(c) for c in ("clang", "gcc", "cc")), reason="no C compiler"
)


# Step response of x' = -a x + u from rest, x(t) = (1 - exp(-a t)) / a, at t = 1.
def step_response(a: float, t: float = 1.0) -> float:
    return (1 - np.exp(-a * t)) / a


# Sessions are exercised on rumoca by default (accurate integrator, fast to build) and
# on openmodelica to prove backend independence.
BACKENDS = [
    pytest.param("rumoca", marks=[needs_rumoca, needs_compiler]),
    pytest.param("openmodelica", marks=needs_omc),
]


@pytest.fixture(scope="module", params=BACKENDS)
def make_session(request, tmp_path_factory):
    """Factory for sessions on a model exported once per backend."""
    model = Model("Driven", [MODEL_FILE])
    fmu = model.export_fmu(tmp_path_factory.mktemp("fmu"), backend=request.param)
    sessions: list[InteractiveSession] = []

    def make(**kwargs):
        kwargs.setdefault("step_size", 0.01)
        kwargs.setdefault("substeps", 20)
        session = InteractiveSession(fmu, **kwargs)
        sessions.append(session)
        return session

    yield make
    for session in sessions:
        session.close()


@pytest.mark.integration
def test_step_matches_analytic_solution(make_session):
    s = make_session()
    s.set(u=1.0)
    s.advance(1.0)
    assert s.time == pytest.approx(1.0)
    assert s.get("x")["x"] == pytest.approx(step_response(2.0), abs=1e-3)
    history = s.history
    assert len(history.time) == 101 and history.time[0] == 0.0
    assert "time" not in history.variables


@pytest.mark.integration
def test_input_can_change_between_steps(make_session):
    s = make_session()
    s.set(u=1.0)
    s.advance(1.0)
    s.set(u=0.0)
    s.advance(1.0)
    # Released at x(1), the state then decays freely with the pole a=2.
    assert s.get("x")["x"] == pytest.approx(step_response(2.0) * np.exp(-2.0), abs=2e-3)


@pytest.mark.integration
def test_on_step_callback_feeds_back_input(make_session):
    s = make_session()
    seen: list[float] = []

    def controller(session):
        seen.append(session.time)
        session.set(u=1.0 if session.get("x")["x"] < 0.2 else 0.0)

    s.advance(0.5, on_step=controller)
    assert len(seen) == 50
    assert s.get("x")["x"] < 0.3  # the bang-bang controller keeps x near its 0.2 target


@pytest.mark.integration
def test_start_values_and_reset(make_session):
    s = make_session(parameters={"a": 4.0})
    s.set(u=1.0)
    s.advance(1.0)
    assert s.get("x")["x"] == pytest.approx(step_response(4.0), abs=1e-3)

    s.reset(parameters={"a": 2.0})
    assert s.time == 0.0
    s.set(u=1.0)
    s.advance(1.0)
    assert s.get("x")["x"] == pytest.approx(step_response(2.0), abs=1e-3)
    assert len(s.history.time) == 101  # history was cleared by the reset


@pytest.mark.integration
def test_fixed_parameter_cannot_change_while_running(make_session):
    s = make_session()
    s.step()
    # a is fixed on OpenModelica and tunable on rumoca, so only check the fixed case.
    fixed = [n for n in ("a", "gain") if n not in s.tunable]
    if not fixed:
        pytest.skip("backend exports tunable parameters")
    with pytest.raises(SessionError, match="reset"):
        s.set({fixed[0]: 3.0})


@pytest.mark.integration
def test_forced_parameter_change_takes_effect(make_session):
    """A mid-run change of `a` must give the same result as the fixed-parameter route."""
    s = make_session()
    s.set(u=1.0)
    s.advance(0.5)
    s.set(a=4.0, force=True)
    assert s.get("a")["a"] == 4.0
    s.advance(0.5)

    # Reference: x' = -a x + 1 with a=2 for 0.5 s, then a=4 for 0.5 s.
    x_half = step_response(2.0, 0.5)
    expected = 0.25 + (x_half - 0.25) * np.exp(-4.0 * 0.5)
    assert s.get("x")["x"] == pytest.approx(expected, abs=1e-3)


@pytest.mark.integration
def test_errors_and_close(make_session):
    s = make_session()
    with pytest.raises(KeyError, match="nope"):
        s.set(nope=1.0)
    with pytest.raises(KeyError, match="nope"):
        s.get("nope")
    with pytest.raises(ValueError):
        s.step(0)
    with pytest.raises(ValueError):
        s.advance(-1.0)
    s.step()
    s.close()
    s.close()  # idempotent
    with pytest.raises(SessionError, match="closed"):
        s.step()


@pytest.mark.integration
def test_live_plot_updates_lines_and_window(make_session):
    s = make_session()
    s.set(u=1.0)
    s.advance(1.0)
    live = LivePlot(["x", "u"], window=0.5)
    fig = live.update(s)
    xdata, ydata = live._lines[0].get_data()
    assert xdata[0] >= 0.5 - 1e-9 and xdata[-1] == pytest.approx(1.0)
    assert ydata[-1] == pytest.approx(s.get("x")["x"])
    assert len(fig.axes) == 2


@pytest.mark.integration
@pytest.mark.filterwarnings("ignore:FigureCanvasAgg is non-interactive")
def test_run_live_headless(make_session):
    matplotlib.use("Agg")
    s = make_session()
    s.set(u=1.0)
    fig = run_live(s, ["x"], 0.5, refresh_every=10, pause=0)
    assert s.time == pytest.approx(0.5)
    assert fig.axes[0].lines[0].get_xdata()[-1] == pytest.approx(0.5)
    import matplotlib.pyplot as plt

    plt.close(fig)


@pytest.mark.integration
@needs_rumoca
@needs_compiler
def test_from_model_builds_and_starts_from_model_parameters():
    model = Model("Driven", [MODEL_FILE], parameters={"a": 4.0})
    with InteractiveSession.from_model(model, backend="rumoca", substeps=20) as s:
        s.set(u=1.0)
        s.advance(1.0)
        assert s.get("x")["x"] == pytest.approx(step_response(4.0), abs=1e-3)


def test_constructor_validation(tmp_path):
    with pytest.raises(FmuError, match="not found"):
        InteractiveSession(tmp_path / "none.fmu")
    with pytest.raises(ValueError):
        InteractiveSession(tmp_path / "none.fmu", step_size=0)
    with pytest.raises(ValueError):
        InteractiveSession(tmp_path / "none.fmu", substeps=0)
