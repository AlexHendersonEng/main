from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration

MU = 3.986004418e14


def _flight_model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAerospace.Tests.FlightDynamics.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, width: int, final: bool = True) -> np.ndarray:
    values = [np.asarray(result[f"{name}[{index}]"]) for index in range(1, width + 1)]
    return np.array([value[-1] for value in values]) if final else np.vstack(values).T


def test_cartesian_point_mass_matches_constant_acceleration(modelica_backend: str):
    result = _flight_model(modelica_backend, "PointMassConstantForce").simulate(
        SimulationOptions(stop_time=4, step_size=0.1),
        backend=modelica_backend,
    )
    acceleration = np.array([3, 2, -1])
    expected_velocity = np.array([4, -2, 1]) + acceleration * 4
    expected_position = np.array([1.0, 2.0, 3.0]) + np.array([4, -2, 1]) * 4
    expected_position = expected_position + 0.5 * acceleration * 4**2

    np.testing.assert_allclose(_array(result, "accelerationNED", 3), acceleration, atol=1e-10)
    np.testing.assert_allclose(_array(result, "velocityNED", 3), expected_velocity, atol=2e-6)
    np.testing.assert_allclose(_array(result, "positionNED", 3), expected_position, atol=2e-5)


def test_flight_path_trim_preserves_state(modelica_backend: str):
    result = _flight_model(modelica_backend, "FlightPathTrim").simulate(
        SimulationOptions(stop_time=5, step_size=0.1),
        backend=modelica_backend,
    )
    expected_velocity = 120 * np.array([math.cos(0.4), math.sin(0.4), 0])
    expected_position = np.array([10, 20, -1000]) + expected_velocity * 5

    assert result["speed"][-1] == pytest.approx(120, abs=1e-8)
    assert result["flightPathAngle"][-1] == pytest.approx(0, abs=1e-9)
    assert result["groundTrack"][-1] == pytest.approx(0.4, abs=1e-9)
    np.testing.assert_allclose(_array(result, "velocityNED", 3), expected_velocity, atol=1e-8)
    np.testing.assert_allclose(_array(result, "positionNED", 3), expected_position, atol=2e-5)


def test_flat_earth_trim_and_load_factor(modelica_backend: str):
    result = _flight_model(modelica_backend, "FlatEarthTrim").simulate(
        SimulationOptions(stop_time=3, step_size=0.1),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "positionNED", 3), [240, 0, -1000], atol=2e-5)
    np.testing.assert_allclose(_array(result, "velocityBody", 3), [80, 0, 0], atol=1e-8)
    np.testing.assert_allclose(_array(result, "angularVelocity", 3), [0, 0, 0], atol=1e-10)
    np.testing.assert_allclose(_array(result, "quaternion", 4), [1, 0, 0, 0], atol=1e-10)
    np.testing.assert_allclose(_array(result, "loadFactor", 3), [0, 0, -1], atol=1e-10)


def test_flat_earth_constant_force_matches_analytic_motion(modelica_backend: str):
    result = _flight_model(modelica_backend, "FlatEarthConstantForce").simulate(
        SimulationOptions(stop_time=3, step_size=0.1),
        backend=modelica_backend,
    )
    acceleration = np.array([2, -1, 0.5])
    expected_velocity = np.array([10, 0, 0]) + acceleration * 3
    expected_position = np.array([10, 0, 0]) * 3 + 0.5 * acceleration * 3**2

    np.testing.assert_allclose(_array(result, "accelerationBody", 3), acceleration, atol=1e-8)
    np.testing.assert_allclose(_array(result, "velocityBody", 3), expected_velocity, atol=2e-6)
    np.testing.assert_allclose(_array(result, "positionNED", 3), expected_position, atol=2e-5)


def test_flat_earth_constant_principal_moment(modelica_backend: str):
    result = _flight_model(modelica_backend, "FlatEarthConstantMoment").simulate(
        SimulationOptions(stop_time=2, step_size=0.05),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "angularAcceleration", 3), [1, 0, 0], atol=1e-8)
    np.testing.assert_allclose(_array(result, "angularVelocity", 3), [2, 0, 0], atol=2e-6)
    assert result["quaternionNorm"][-1] == pytest.approx(1, abs=2e-7)
    expected_quaternion = np.array([math.cos(1), math.sin(1), 0, 0])
    quaternion = _array(result, "quaternion", 4)
    assert abs(float(np.dot(quaternion, expected_quaternion))) == pytest.approx(1, abs=2e-6)


def test_spherical_ballistic_energy_and_momentum_invariants(modelica_backend: str):
    result = _flight_model(modelica_backend, "SphericalBallistic").simulate(
        SimulationOptions(stop_time=120, step_size=0.5, tolerance=1e-9),
        backend=modelica_backend,
    )
    energy = np.asarray(result["energy"])
    momentum = _array(result, "angularMomentum", 3, final=False)
    expected_energy = 0.5 * 7500**2 - MU / 7_000_000
    expected_momentum = np.array([0, 0, 7_000_000 * 7500])

    assert energy[0] == pytest.approx(expected_energy, rel=2e-10)
    assert np.max(np.abs(energy - energy[0])) < 2
    np.testing.assert_allclose(momentum[0], expected_momentum, rtol=2e-10, atol=1e-6)
    assert np.max(np.linalg.norm(momentum - momentum[0], axis=1)) < 2e5
    assert result["quaternionNorm"][-1] == pytest.approx(1, abs=2e-7)


def test_spherical_local_limit_matches_flat_short_time(modelica_backend: str):
    result = _flight_model(modelica_backend, "SphericalLocalLimit").simulate(
        SimulationOptions(stop_time=1, step_size=0.02),
        backend=modelica_backend,
    )
    velocity_body = _array(result, "velocityBody", 3)
    position_ecef = _array(result, "positionECEF", 3)
    velocity_ecef = _array(result, "velocityECEF", 3)

    assert velocity_body[0] == pytest.approx(52, abs=2e-3)
    assert velocity_body[1] == pytest.approx(0, abs=2e-3)
    assert velocity_body[2] == pytest.approx(9.80665, abs=2e-3)
    assert position_ecef[0] == pytest.approx(6_378_137 - 0.5 * 9.80665, abs=5e-3)
    assert position_ecef[2] == pytest.approx(51, abs=5e-3)
    assert velocity_ecef[0] == pytest.approx(-9.80665, abs=2e-3)
    assert velocity_ecef[2] == pytest.approx(52, abs=2e-3)
