from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def _subsystem_model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAerospace.Tests.Subsystems.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, width: int, final: bool = True) -> np.ndarray:
    values = [np.asarray(result[f"{name}[{index}]"]) for index in range(1, width + 1)]
    return np.array([value[-1] for value in values]) if final else np.vstack(values).T


def test_aerodynamic_scaling_and_derivatives(modelica_backend: str):
    result = _subsystem_model(modelica_backend, "AerodynamicsValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "forceBody", 3), [-500, 100, -2500])
    np.testing.assert_allclose(_array(result, "momentBody", 3), [400, -300, 800])

    control = np.array([0.05, -0.1, 0.02])
    force_control = np.array([[0, 0.2, 0], [0.1, 0, 0.3], [0, -0.5, 0]])
    moment_control = np.array([[0.8, 0, 0.1], [0, -1.2, 0], [-0.1, 0, -0.7]])
    expected_force = np.array([-0.02, 0, 0])
    expected_force += np.array([-0.1, 0, -4]) * 0.1
    expected_force += np.array([0, -0.8, 0]) * -0.05
    expected_force += np.diag([0.1, 0.2, 0.3]) @ np.array([0.004, 0.002, -0.004])
    expected_force += force_control @ control
    expected_moment = np.array([0, -1, 0]) * 0.1
    expected_moment += np.array([-0.1, 0, 0.2]) * -0.05
    expected_moment += np.diag([0.4, 0.5, 0.6]) @ np.array([0.004, 0.002, -0.004])
    expected_moment += moment_control @ control
    np.testing.assert_allclose(_array(result, "derivativeForce", 3), expected_force)
    np.testing.assert_allclose(_array(result, "derivativeMoment", 3), expected_moment)


def test_msl_aerodynamic_tables(modelica_backend: str):
    if modelica_backend == "rumoca":
        pytest.skip("Rumoca does not yet support the MSL native table constructor")
    result = _subsystem_model(modelica_backend, "AerodynamicTableValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    probe = 1e-7
    assert result["clampedLift"][-1] == pytest.approx(0.8 + probe)
    assert result["clampedDrag"][-1] == pytest.approx(0.12 + probe)
    assert result["clampedPitchingMoment"][-1] == pytest.approx(-0.1 + probe)
    assert result["linearLift"][-1] == pytest.approx(1.2 + probe)
    assert result["linearDrag"][-1] == pytest.approx(0.17 + probe)
    assert result["linearPitchingMoment"][-1] == pytest.approx(-0.15 + probe)
    assert result["interpolatedValue"][-1] == pytest.approx(1 + probe)


def test_propulsion_spool_thrust_and_fuel_conservation(modelica_backend: str):
    result = _subsystem_model(modelica_backend, "PropulsionValidation").simulate(
        SimulationOptions(stop_time=5, step_size=0.05, tolerance=1e-9),
        backend=modelica_backend,
    )
    elapsed = 4
    expected_spool = 0.8 * (1 - math.exp(-elapsed / 0.5))
    expected_consumed = 0.2 * 0.8 * (elapsed - 0.5 * (1 - math.exp(-elapsed / 0.5)))

    np.testing.assert_allclose(
        _array(result, "sourceForce", 3),
        [1000 / math.sqrt(2), 1000 / math.sqrt(2), 0],
        atol=1e-8,
    )
    assert result["spool"][-1] == pytest.approx(expected_spool, abs=3e-6)
    assert result["engineThrust"][-1] == pytest.approx(2000 * expected_spool, abs=6e-3)
    assert result["fuelFlow"][-1] == pytest.approx(0.2 * expected_spool, abs=1e-6)
    assert result["consumedFuelMass"][-1] == pytest.approx(expected_consumed, abs=2e-5)
    assert result["fuelMass"][-1] + result["consumedFuelMass"][-1] == pytest.approx(10, abs=1e-9)
    assert result["propellerThrust"][-1] == pytest.approx(800)
    expected_jet = 10000 * 0.75 * 0.5**0.7 * (1 - 0.25 * 0.8)
    assert result["jetThrust"][-1] == pytest.approx(expected_jet)


def test_actuator_limits_rate_deadband_and_failure(modelica_backend: str):
    result = _subsystem_model(modelica_backend, "ActuatorValidation").simulate(
        SimulationOptions(stop_time=7, step_size=0.02),
        backend=modelica_backend,
    )
    time = np.asarray(result["time"])
    position = np.asarray(result["position"])
    rate = np.asarray(result["positionRate"])
    failed_position = np.asarray(result["failedPosition"])

    assert position[np.argmin(np.abs(time - 0.8))] == pytest.approx(0, abs=1e-8)
    assert position[np.argmin(np.abs(time - 2.0))] == pytest.approx(0.5, abs=3e-3)
    assert position[np.argmin(np.abs(time - 3.2))] == pytest.approx(1, abs=3e-3)
    assert position[-1] == pytest.approx(1, abs=3e-3)
    assert failed_position[-1] == pytest.approx(-0.25, abs=3e-3)
    assert np.max(position) <= 1 + 2e-4
    assert np.min(position) >= -1 - 2e-4
    assert np.max(np.abs(rate)) <= 0.5 + 2e-5


def test_ideal_and_configurable_sensor_behavior(modelica_backend: str):
    result = _subsystem_model(modelica_backend, "SensorValidation").simulate(
        SimulationOptions(stop_time=3, step_size=0.05),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "idealMeasurement", 3), [1, 2, 3])
    np.testing.assert_allclose(
        _array(result, "idealAirDataMeasurement", 4),
        [120, 0.1, -0.02, 1500],
    )
    np.testing.assert_allclose(_array(result, "idealAcceleration", 3), [1, -2, 3])
    np.testing.assert_allclose(_array(result, "idealRates", 3), [0.1, 0.2, -0.3])
    np.testing.assert_allclose(_array(result, "gpsPosition", 3), [6378138, 12, 23])
    np.testing.assert_allclose(_array(result, "gpsVelocity", 3), [100.1, -19.8, 5.3])
    assert result["measuredAltitude"][-1] == pytest.approx(104)
    expected_quaternion = np.array([1, 0.1, 0, 0]) / math.sqrt(1.01)
    np.testing.assert_allclose(
        _array(result, "measuredQuaternion", 4),
        expected_quaternion,
        atol=1e-10,
    )
    assert result["delayedSignal"][-1] == pytest.approx(1 - math.exp(-4), abs=3e-5)


def test_nonideal_sensor_trace_is_repeatable_and_quantized(modelica_backend: str):
    options = SimulationOptions(stop_time=4, step_size=0.05)
    first = _subsystem_model(modelica_backend, "SensorValidation").simulate(
        options,
        backend=modelica_backend,
    )
    second = _subsystem_model(modelica_backend, "SensorValidation").simulate(
        options,
        backend=modelica_backend,
    )
    first_trace = _array(first, "vectorMeasurement", 3, final=False)
    second_trace = _array(second, "vectorMeasurement", 3, final=False)
    np.testing.assert_allclose(first_trace, second_trace, atol=1e-12, rtol=0)
    np.testing.assert_allclose(first_trace / 0.05, np.round(first_trace / 0.05), atol=1e-10)
