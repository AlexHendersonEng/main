from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAutomotive.Tests.Planar.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _value(result, *names: str) -> float:
    for name in names:
        if name in result.variables:
            return float(result[name][-1])
    raise KeyError(f"none of the result variables are available: {names}")


def test_steering_ratio_ackermann_and_first_order_response(modelica_backend: str):
    result = _model(modelica_backend, "SteeringValidation").simulate(
        SimulationOptions(stop_time=1, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    expected_dynamic = 0.3 * (1 - math.exp(-2))
    tangent = math.tan(0.3)
    expected_left = math.atan(2.7 * tangent / (2.7 - 0.8 * tangent))
    expected_right = math.atan(2.7 * tangent / (2.7 + 0.8 * tangent))
    ratio_angle = (
        _value(result, "ratioAngle", "ratio.roadWheelAngle")
        if any(name in result.variables for name in ("ratioAngle", "ratio.roadWheelAngle"))
        else result["ratioIntegral"][-1]
    )
    assert ratio_angle == pytest.approx(0.4, abs=2e-9)
    assert result["dynamicAngle"][-1] == pytest.approx(expected_dynamic, abs=2e-6)
    wheel_angles = []
    for index in range(1, 5):
        candidates = (f"wheelAngles[{index}]", f"ackermann.wheelAngles[{index}]")
        wheel_angles.append(
            _value(result, *candidates)
            if any(name in result.variables for name in candidates)
            else float(result[f"wheelAngleIntegral[{index}]"][-1])
        )
    assert wheel_angles[0] == pytest.approx(expected_left, abs=1e-12)
    assert wheel_angles[1] == pytest.approx(expected_right, abs=1e-12)
    assert wheel_angles[2] == pytest.approx(0, abs=1e-12)
    assert wheel_angles[3] == pytest.approx(0, abs=1e-12)
    assert wheel_angles[0] > wheel_angles[1]
    negative_angles = np.array(
        [result[f"negativeWheelAngleIntegral[{index}]"][-1] for index in range(1, 5)]
    )
    np.testing.assert_allclose(
        negative_angles,
        [-wheel_angles[1], -wheel_angles[0], 0, 0],
        atol=2e-9,
    )


def test_kinematic_bicycle_matches_constant_curvature_solution(
    modelica_backend: str,
):
    duration = 4
    speed = 10
    steering = 0.2
    wheelbase = 2.7
    yaw_rate = speed * math.tan(steering) / wheelbase
    radius = speed / yaw_rate
    result = _model(modelica_backend, "KinematicCircle").simulate(
        SimulationOptions(stop_time=duration, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    expected_yaw = yaw_rate * duration
    assert result["yawRate"][-1] == pytest.approx(yaw_rate, abs=1e-11)
    assert result["yaw"][-1] == pytest.approx(expected_yaw, abs=2e-6)
    assert result["positionX"][-1] == pytest.approx(
        radius * math.sin(expected_yaw),
        abs=2e-5,
    )
    assert result["positionY"][-1] == pytest.approx(
        radius * (1 - math.cos(expected_yaw)),
        abs=2e-5,
    )


def test_dynamic_bicycle_reaches_linear_steady_state(modelica_backend: str):
    mass = 1500
    inertia = 2500
    speed = 15
    front_distance = 1.2
    rear_distance = 1.5
    front_stiffness = 100000
    rear_stiffness = 120000
    steering = 0.05
    matrix = np.array(
        [
            [
                -(front_stiffness + rear_stiffness) / (mass * speed),
                (-front_distance * front_stiffness + rear_distance * rear_stiffness)
                / (mass * speed)
                - speed,
            ],
            [
                (-front_distance * front_stiffness + rear_distance * rear_stiffness)
                / (inertia * speed),
                -(front_distance**2 * front_stiffness + rear_distance**2 * rear_stiffness)
                / (inertia * speed),
            ],
        ]
    )
    forcing = np.array(
        [
            front_stiffness * steering / mass,
            front_distance * front_stiffness * steering / inertia,
        ]
    )
    expected_lateral_velocity, expected_yaw_rate = np.linalg.solve(matrix, -forcing)
    result = _model(modelica_backend, "DynamicStep").simulate(
        SimulationOptions(stop_time=8, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert result["lateralVelocity"][-1] == pytest.approx(
        expected_lateral_velocity,
        abs=2e-4,
    )
    assert result["yawRate"][-1] == pytest.approx(expected_yaw_rate, abs=2e-4)
    assert result["lateralAcceleration"][-1] == pytest.approx(
        speed * expected_yaw_rate,
        abs=3e-3,
    )
    assert result["frontSlipAngle"][-1] > 0
    assert result["rearSlipAngle"][-1] > 0


def test_double_track_straight_motion_and_load_transfer(modelica_backend: str):
    duration = 4
    result = _model(modelica_backend, "DoubleTrackStraight").simulate(
        SimulationOptions(stop_time=duration, step_size=0.05),
        backend=modelica_backend,
    )
    assert result["longitudinalAcceleration"][-1] == pytest.approx(2, abs=1e-10)
    assert result["longitudinalVelocity"][-1] == pytest.approx(18, abs=2e-6)
    assert result["positionX"][-1] == pytest.approx(56, abs=2e-5)
    assert result["yawRate"][-1] == pytest.approx(0, abs=1e-12)
    loads = np.array([result[f"normalLoad[{index}]"][-1] for index in range(1, 5)])
    assert np.sum(loads) == pytest.approx(1000 * 9.80665, abs=1e-8)
    assert loads[0] == pytest.approx(loads[1], abs=1e-10)
    assert loads[2] == pytest.approx(loads[3], abs=1e-10)
    assert loads[0] < 1000 * 9.80665 * 1.5 / (2 * 2.7)


def test_balanced_corner_preserves_yaw_and_conserves_load(modelica_backend: str):
    result = _model(modelica_backend, "DoubleTrackBalancedCorner").simulate(
        SimulationOptions(stop_time=2, step_size=0.02),
        backend=modelica_backend,
    )
    assert result["lateralAcceleration"][-1] == pytest.approx(2.4, abs=1e-10)
    assert result["yawRate"][-1] == pytest.approx(0, abs=1e-10)
    assert result["loadSum"][-1] == pytest.approx(1500 * 9.80665, abs=1e-8)
    assert result["normalLoad[1]"][-1] < result["normalLoad[2]"][-1]
    assert result["normalLoad[3]"][-1] < result["normalLoad[4]"][-1]


def test_braking_turn_is_bounded_and_turns_left(modelica_backend: str):
    result = _model(modelica_backend, "BrakingTurn").simulate(
        SimulationOptions(stop_time=3, step_size=0.01),
        backend=modelica_backend,
    )
    assert 10 < result["longitudinalVelocity"][-1] < 20
    assert result["yawRate"][-1] > 0
    assert result["yaw"][-1] > 0
    assert np.all(np.isfinite(result["lateralVelocity"]))


def test_planar_models_declare_physical_parameter_guards():
    sources = [
        PACKAGE_ROOT / "Steering" / "SteeringRatio.mo",
        PACKAGE_ROOT / "Steering" / "FirstOrderSteering.mo",
        PACKAGE_ROOT / "VehicleDynamics" / "Planar" / "DynamicBicycle.mo",
        PACKAGE_ROOT / "VehicleDynamics" / "Planar" / "DoubleTrack.mo",
    ]
    text = "\n".join(path.read_text(encoding="utf-8") for path in sources)
    for guard in (
        "assert(ratio > 0,",
        "assert(timeConstant > 0,",
        "assert(vehicle.mass > 0,",
        "assert(yawInertia > 0,",
        "assert(frontDistance > 0 and rearDistance > 0,",
    ):
        assert guard in text
