from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAutomotive.Tests.DriversSensorsControl.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def test_static_commands_ideal_sensors_and_brake_blending(modelica_backend: str):
    result = _model(modelica_backend, "ComponentValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["openPropulsion"][-1] == pytest.approx(1)
    assert result["openBrake"][-1] == pytest.approx(0)
    open_steering = (
        result["openSteering"][-1]
        if "openSteering" in result.variables
        else result["openSteeringIntegral"][-1] / 0.1
    )
    steering = (
        result["steering"][-1]
        if "steering" in result.variables
        else result["steeringIntegral"][-1] / 0.1
    )
    assert open_steering == pytest.approx(0.5)
    assert result["brake"][-1] == pytest.approx(0.7)
    assert result["propulsion"][-1] == pytest.approx(0.18)
    assert steering == pytest.approx(0.05)
    np.testing.assert_allclose(
        [result[f"sensedWheelSpeed[{index}]"][-1] for index in range(1, 5)],
        [10, 11, 12, 13],
    )
    assert result["sensedYawRate"][-1] == pytest.approx(0.3)
    assert result["regenerativeCommand"][-1] == pytest.approx(-0.3)
    assert result["frictionCommand"][-1] == pytest.approx(0.5)
    assert result["achievedBrake"][-1] == pytest.approx(0.8)


def test_deterministic_sensor_matches_first_order_reference(modelica_backend: str):
    result = _model(modelica_backend, "DeterministicSensorValidation").simulate(
        SimulationOptions(stop_time=2, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    expected = 7 - 6 * math.exp(-4)
    assert result["idealValue"][-1] == pytest.approx(7, abs=1e-12)
    assert result["measurement"][-1] == pytest.approx(expected, abs=2e-6)
    assert result["measurementError"][-1] == pytest.approx(expected - 3, abs=2e-6)


def test_speed_controller_tracks_acceleration_and_deceleration_targets(
    modelica_backend: str,
):
    result = _model(modelica_backend, "SpeedTracking").simulate(
        SimulationOptions(stop_time=20, step_size=0.02, tolerance=1e-8),
        backend=modelica_backend,
    )
    time = np.asarray(result.time)
    speed = np.asarray(result["speed"])
    propulsion = np.asarray(result["propulsionCommand"])
    brake = np.asarray(result["brakeCommand"])
    before_step = np.argmin(np.abs(time - 9.5))
    assert speed[before_step] == pytest.approx(15, abs=0.35)
    assert speed[-1] == pytest.approx(8, abs=0.25)
    assert np.max(propulsion[:before_step]) == pytest.approx(1, abs=1e-8)
    assert np.max(brake[before_step:]) > 0.2
    assert np.max(np.abs(result["integralState"])) < 1.5


def test_look_ahead_path_tracking_converges(modelica_backend: str):
    result = _model(modelica_backend, "PathTracking").simulate(
        SimulationOptions(stop_time=10, step_size=0.01, tolerance=1e-9),
        backend=modelica_backend,
    )
    lateral_error = np.asarray(result["lateralError"])
    steering = np.asarray(result["steeringCommand"])
    assert abs(lateral_error[-1]) < 0.08
    assert abs(result["yaw"][-1]) < 0.03
    assert np.max(np.abs(steering)) <= 0.4 + 1e-10
    assert np.all(np.isfinite(result["lateralPosition"]))


def test_abs_and_traction_control_reduce_slip(modelica_backend: str):
    result = _model(modelica_backend, "SlipControlValidation").simulate(
        SimulationOptions(stop_time=3, step_size=0.005, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert abs(result["controlledBrakeSlip"][-1]) < abs(result["uncontrolledBrakeSlip"][-1])
    assert result["controlledDriveSlip"][-1] < result["uncontrolledDriveSlip"][-1]
    assert result["absActive"][-1] == pytest.approx(1)
    assert result["tractionActive"][-1] == pytest.approx(1)
    assert 0.05 <= result["brakeCommand"][-1] < 1
    assert 0.05 <= result["propulsionCommand"][-1] < 1


def test_yaw_stability_control_rejects_disturbance(modelica_backend: str):
    result = _model(modelica_backend, "YawControlValidation").simulate(
        SimulationOptions(stop_time=8, step_size=0.01, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert abs(result["controlledYawRate"][-1]) < 0.2 * abs(result["uncontrolledYawRate"][-1])
    assert result["yawMomentCommand"][-1] < 0
    assert result["controllerActive"][-1] == pytest.approx(1)


def test_driver_sensor_and_control_parameter_guards():
    sources = [
        PACKAGE_ROOT / "Drivers" / "SpeedController.mo",
        PACKAGE_ROOT / "Drivers" / "LookAheadSteering.mo",
        PACKAGE_ROOT / "Sensors" / "DeterministicSensor.mo",
        PACKAGE_ROOT / "Control" / "AntiLockBraking.mo",
        PACKAGE_ROOT / "Control" / "TractionControl.mo",
        PACKAGE_ROOT / "Control" / "YawStabilityControl.mo",
    ]
    text = "\n".join(path.read_text(encoding="utf-8") for path in sources)
    for guard in (
        "assert(antiWindupGain > 0,",
        "assert(lookAheadTime > 0,",
        "assert(timeConstant > 0,",
        "assert(lockedSlip > activationSlip,",
        "assert(maximumSlip > activationSlip,",
        "assert(maximumYawMoment >= 0,",
    ):
        assert guard in text
