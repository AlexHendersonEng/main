from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration

EARTH_SEMI_MAJOR_AXIS = 6_378_137.0
EARTH_ECCENTRICITY_SQUARED = 6.69437999014e-3


def _example(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAerospace.Examples.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, width: int, final: bool = True) -> np.ndarray:
    values = [np.asarray(result[f"{name}[{index}]"]) for index in range(1, width + 1)]
    return np.array([value[-1] for value in values]) if final else np.vstack(values).T


def _atmosphere_reference(geometric_altitude: float) -> tuple[float, float, float, float]:
    geopotential_radius = 6_356_766.0
    altitude = geopotential_radius * geometric_altitude / (geopotential_radius + geometric_altitude)
    temperature = 288.15 - 0.0065 * altitude
    pressure = 101_325 * (288.15 / temperature) ** (9.80665 / (287.05287 * -0.0065))
    density = pressure / (287.05287 * temperature)
    speed_of_sound = math.sqrt(1.4 * 287.05287 * temperature)
    return temperature, pressure, density, speed_of_sound


def _ecef_reference(latitude: float, longitude: float, altitude: float) -> np.ndarray:
    radius = EARTH_SEMI_MAJOR_AXIS / math.sqrt(
        1 - EARTH_ECCENTRICITY_SQUARED * math.sin(latitude) ** 2
    )
    return np.array(
        [
            (radius + altitude) * math.cos(latitude) * math.cos(longitude),
            (radius + altitude) * math.cos(latitude) * math.sin(longitude),
            (radius * (1 - EARTH_ECCENTRICITY_SQUARED) + altitude) * math.sin(latitude),
        ]
    )


def test_atmosphere_geodesy_example_matches_independent_reference(
    modelica_backend: str,
):
    result = _example(modelica_backend, "AtmosphereGeodesy").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1, tolerance=1e-9),
        backend=modelica_backend,
    )
    expected_atmosphere = _atmosphere_reference(10_000)
    actual_atmosphere = np.array(
        [
            result["temperature"][0],
            result["pressure"][0],
            result["density"][0],
            result["speedOfSound"][0],
        ]
    )
    np.testing.assert_allclose(actual_atmosphere, expected_atmosphere, rtol=2e-10)

    latitude = math.radians(45)
    longitude = math.radians(-93)
    expected_ecef = _ecef_reference(latitude, longitude, 10_000)
    np.testing.assert_allclose(_array(result, "positionECEF", 3), expected_ecef, atol=2e-6)
    np.testing.assert_allclose(
        _array(result, "recoveredGeodetic", 3),
        [latitude, longitude, 10_000],
        atol=1e-6,
    )


def test_ballistic_example_matches_closed_form_and_conserves_energy(
    modelica_backend: str,
):
    result = _example(modelica_backend, "BallisticTrajectory").simulate(
        SimulationOptions(stop_time=20, step_size=0.1, tolerance=1e-9),
        backend=modelica_backend,
    )
    duration = 20
    gravity = 9.80665
    expected_position = np.array([50 * duration, 0, -100 * duration + 0.5 * gravity * duration**2])
    expected_velocity = np.array([50, 0, -100 + gravity * duration])
    np.testing.assert_allclose(_array(result, "positionNED", 3), expected_position, atol=2e-5)
    np.testing.assert_allclose(_array(result, "velocityNED", 3), expected_velocity, atol=2e-6)
    energy = np.asarray(result["specificMechanicalEnergy"])
    assert np.max(np.abs(energy - 6250)) < 1e-8


def test_longitudinal_aircraft_example_trim_and_throttle_response(
    modelica_backend: str,
):
    result = _example(modelica_backend, "LongitudinalAircraft").simulate(
        SimulationOptions(stop_time=15, step_size=0.05, tolerance=1e-8),
        backend=modelica_backend,
    )
    assert result["speed"][0] == pytest.approx(70)
    assert result["flightPathAngle"][0] == pytest.approx(0)
    assert result["altitudeOutput"][0] == pytest.approx(1000)
    assert result["thrust"][0] == pytest.approx(1525.1970723, rel=2e-9)
    assert result["speed"][-1] == pytest.approx(71.625867, abs=2e-3)
    assert result["altitudeOutput"][-1] == pytest.approx(1038.902, abs=2e-2)
    assert result["distanceNorth"][-1] == pytest.approx(1073.910, abs=2e-2)
    assert result["thrust"][-1] == pytest.approx(2275.1970723, abs=2e-3)


def test_six_degree_of_freedom_aircraft_example_is_damped_and_normalized(
    modelica_backend: str,
):
    result = _example(modelica_backend, "SixDegreeOfFreedomAircraft").simulate(
        SimulationOptions(stop_time=8, step_size=0.02, tolerance=1e-8),
        backend=modelica_backend,
    )
    rates = _array(result, "angularVelocityBody", 3, final=False)
    attitude = _array(result, "euler321", 3, final=False)
    norm = np.asarray(result["quaternionNorm"])
    airspeed = np.asarray(result["airspeed"])

    assert rates[0, 0] == pytest.approx(0)
    assert np.max(rates[:, 0]) > 0.2
    assert abs(rates[-1, 0]) < 0.005
    assert np.max(np.abs(rates[-1])) < 0.01
    assert 0.07 < attitude[-1, 0] < 0.1
    assert np.max(np.abs(norm - 1)) < 3e-7
    assert np.max(np.abs(airspeed - 80)) < 0.05
    np.testing.assert_allclose(
        _array(result, "positionNED", 3),
        [639.43964, 21.49827, -998.95502],
        atol=3e-2,
    )


def test_dryden_gust_response_has_repeatable_bounded_regression(
    modelica_backend: str,
):
    result = _example(modelica_backend, "DrydenGustResponse").simulate(
        SimulationOptions(stop_time=40, step_size=0.05, tolerance=1e-8),
        backend=modelica_backend,
    )
    turbulence = _array(result, "turbulenceBody", 3, final=False)
    response = np.asarray(result["verticalVelocity"])
    assert response[-1] == pytest.approx(0.533590, abs=2e-4)
    assert np.min(response) == pytest.approx(-0.970284, abs=2e-4)
    assert np.max(response) == pytest.approx(0.964691, abs=2e-4)
    assert np.max(np.abs(turbulence[:, 0])) < 1
    assert 3 < np.max(np.abs(turbulence[:, 1])) < 3.5
    assert 2 < np.max(np.abs(turbulence[:, 2])) < 2.5


def test_waypoint_following_example_enters_acceptance_sphere(
    modelica_backend: str,
):
    result = _example(modelica_backend, "WaypointFollowing").simulate(
        SimulationOptions(stop_time=30, step_size=0.05, tolerance=1e-8),
        backend=modelica_backend,
    )
    distance = np.asarray(result["distanceToWaypoint"])
    reached = np.asarray(result["waypointReached"])
    position = _array(result, "positionNED", 3, final=False)
    assert distance[0] == pytest.approx(math.sqrt(1_250_000))
    assert np.min(distance) < 2
    assert np.max(reached) == pytest.approx(1)
    closest = position[np.argmin(distance)]
    np.testing.assert_allclose(closest[:2], [1000, 500], atol=2)
    assert np.max(np.abs(position[:, 2])) < 1e-9
