"""Dashboard API and callback tests, using real FMUs when a compiler is available."""

from __future__ import annotations

import shutil
from importlib.util import find_spec
from pathlib import Path

import pytest

from polaris import Model
from polaris.interactive import InteractiveSession, create_dashboard

pytestmark = pytest.mark.skipif(find_spec("dash") is None, reason="install the dashboard extra")

MODEL_FILE = Path(__file__).parent / "models" / "Driven.mo"
needs_rumoca = pytest.mark.skipif(shutil.which("rumoca") is None, reason="rumoca not installed")
needs_omc = pytest.mark.skipif(shutil.which("omc") is None, reason="omc not installed")
needs_compiler = pytest.mark.skipif(
    not any(shutil.which(c) for c in ("clang", "gcc", "cc")), reason="no C compiler"
)


@pytest.fixture
def dashboard(tmp_path):
    model = Model("Driven", [MODEL_FILE])
    fmu = model.export_fmu(tmp_path / "Driven.fmu", backend="rumoca")
    session = InteractiveSession(fmu, step_size=0.01, substeps=20)
    app = create_dashboard(
        session,
        {"u": (0.0, 2.0), "a": (0.5, 5.0)},
        variables=("x",),
        force_fixed_parameters=True,
    )
    yield app, session
    session.close()


def _callback(app, *, action: str, clicks: int, duration: float, controls: dict[str, float]):
    """Send a real Dash callback request so layout wiring and state order are verified."""
    key = next(key for key in app.callback_map if "polaris-trajectory.figure" in key)
    inputs = [
        {"id": "polaris-step", "property": "n_clicks", "value": clicks if action == "step" else 0},
        {
            "id": "polaris-reset",
            "property": "n_clicks",
            "value": clicks if action == "reset" else 0,
        },
    ]
    states = [
        {"id": "polaris-duration", "property": "value", "value": duration},
        *[
            {"id": f"control-{name}", "property": "value", "value": value}
            for name, value in controls.items()
        ],
    ]
    payload = {
        "output": key,
        "outputs": [
            {"id": "polaris-trajectory", "property": "figure"},
            {"id": "polaris-status", "property": "children"},
        ],
        "inputs": inputs,
        "state": states,
        "changedPropIds": [f"polaris-{action}.n_clicks"],
    }
    return app.server.test_client().post("/_dash-update-component", json=payload)


@needs_rumoca
@needs_compiler
def test_dashboard_step_and_reset_control_fmu(dashboard):
    app, session = dashboard
    response = _callback(
        app,
        action="step",
        clicks=1,
        duration=0.5,
        controls={"u": 1.0, "a": 2.0},
    )
    assert response.status_code == 200, response.get_data(as_text=True)
    assert session.time == pytest.approx(0.5)
    assert response.json["response"]["polaris-status"]["children"] == "t=0.5"

    response = _callback(
        app,
        action="reset",
        clicks=1,
        duration=0.5,
        controls={"u": 1.0, "a": 4.0},
    )
    assert response.status_code == 200, response.get_data(as_text=True)
    assert session.time == 0.0
    assert session.get("a")["a"] == pytest.approx(4.0)


@needs_omc
def test_dashboard_validates_control_ranges(tmp_path):
    fmu = Model("Driven", [MODEL_FILE]).export_fmu(tmp_path / "Driven.fmu", backend="openmodelica")
    with InteractiveSession(fmu) as session:
        with pytest.raises(ValueError, match="bounds"):
            create_dashboard(session, {"u": (1.0, 1.0)})
        with pytest.raises(ValueError, match="fixed"):
            create_dashboard(session, {"a": (1.0, 5.0)})
