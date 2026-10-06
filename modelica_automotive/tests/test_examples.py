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


def test_tire_sweep_respects_force_limits_and_symmetry(modelica_backend: str):
    model = Model(
        "ModelicaAutomotive.Examples.TireSweep",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=4, step_size=0.02),
        backend=modelica_backend,
    )
    expected_stiffness = {
        "linearForce": (80000, 60000),
        "fialaForce": (100000, 80000),
        "magicFormulaForce": (90000, 70000),
    }
    slip_ratio = np.asarray(result["slipRatio"])
    slip_angle = np.asarray(result["slipAngle"])
    small_slip_index = np.argmin(np.abs(np.asarray(result.time) - 2.02))
    for name in ("linearForce", "fialaForce", "magicFormulaForce"):
        forces = np.vstack((result[f"{name}[1]"], result[f"{name}[2]"])).T
        assert np.max(np.linalg.norm(forces, axis=1)) <= 3600 + 2e-6
        np.testing.assert_allclose(forces[-1], -forces[0], atol=2e-6)
        midpoint = np.argmin(np.abs(np.asarray(result.time) - 2))
        np.testing.assert_allclose(forces[midpoint], [0, 0], atol=2e-8)
        longitudinal_stiffness, lateral_stiffness = expected_stiffness[name]
        assert forces[small_slip_index, 0] / slip_ratio[small_slip_index] == pytest.approx(
            longitudinal_stiffness,
            rel=0.04,
        )
        assert forces[small_slip_index, 1] / slip_angle[small_slip_index] == pytest.approx(
            lateral_stiffness,
            rel=0.04,
        )


def test_split_friction_braking_slows_vehicle_and_generates_yaw_moment(
    modelica_backend: str,
):
    model = Model(
        "ModelicaAutomotive.Examples.SplitFrictionBraking",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=3, step_size=0.005, tolerance=1e-8),
        backend=modelica_backend,
    )
    speed = np.asarray(result["speed"])
    left_force = np.asarray(result["leftLongitudinalForce"])
    right_force = np.asarray(result["rightLongitudinalForce"])
    yaw_moment = np.asarray(result["yawMoment"])
    assert speed[0] == pytest.approx(20)
    assert 5 < speed[-1] < 20
    assert np.min(left_force) < np.min(right_force)
    assert np.max(np.abs(yaw_moment)) > 1000
    assert np.all(np.isfinite(speed))


def test_bicycle_models_converge_at_low_speed(modelica_backend: str):
    model = Model(
        "ModelicaAutomotive.Examples.BicycleComparison",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=8, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert result["kinematicYaw"][-1] > 0
    assert result["dynamicYaw"][-1] > 0
    assert result["dynamicYaw"][-1] == pytest.approx(
        result["kinematicYaw"][-1],
        rel=0.08,
    )
    kinematic_position = np.array(
        [result["kinematicPosition[1]"][-1], result["kinematicPosition[2]"][-1]]
    )
    dynamic_position = np.array(
        [result["dynamicPosition[1]"][-1], result["dynamicPosition[2]"][-1]]
    )
    assert np.linalg.norm(dynamic_position - kinematic_position) < 2


def test_planar_maneuver_response_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaAutomotive.Examples.PlanarManeuver",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=10, step_size=0.01, tolerance=1e-8),
        backend=modelica_backend,
    )
    steering_angle = np.asarray(result["steeringAngle"])
    yaw_rate = np.asarray(result["yawRate"])
    lateral_acceleration = np.asarray(result["lateralAcceleration"])
    assert np.max(np.abs(steering_angle)) <= 0.2
    assert np.max(np.abs(yaw_rate)) < 0.5
    assert np.max(np.abs(lateral_acceleration)) < 8
    assert np.all(np.isfinite(result["position[1]"]))
    assert np.all(np.isfinite(result["position[2]"]))
