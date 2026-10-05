from __future__ import annotations

import numpy as np
import pytest
from polaris import Model, SimulationOptions
from scipy.integrate import solve_ivp

from conftest import library_model_files

pytestmark = pytest.mark.integration


def test_longitudinal_drive_matches_independent_integration(modelica_backend: str):
    model = Model(
        "ModelicaAutomotive.Examples.LongitudinalDrive",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=20, step_size=0.05, tolerance=1e-9),
        backend=modelica_backend,
    )

    mass = 1500
    drag_factor = 0.5 * 1.225 * 0.72
    rolling_force = 0.012 * mass * 9.80665
    regularization = 0.05

    def dynamics(time: float, state: np.ndarray) -> list[float]:
        speed = state[1]
        tire_force = 3500 if time < 5 else 0
        rolling = rolling_force * speed / np.sqrt(speed * speed + regularization**2)
        drag = drag_factor * speed * abs(speed)
        return [speed, (tire_force - rolling - drag) / mass]

    reference = solve_ivp(
        dynamics,
        (0, 20),
        (0, 10),
        rtol=1e-11,
        atol=1e-12,
        max_step=0.005,
    )
    expected_position, expected_speed = reference.y[:, -1]
    assert result["position"][-1] == pytest.approx(expected_position, abs=3e-3)
    assert result["speed"][-1] == pytest.approx(expected_speed, abs=3e-4)
    assert np.max(result["speed"]) < 22
    assert result["speed"][-1] > 10
