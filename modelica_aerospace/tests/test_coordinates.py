from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration

WGS84_A = 6378137.0
WGS84_F = 1 / 298.257223563
WGS84_E2 = WGS84_F * (2 - WGS84_F)
EARTH_RATE = 7.292115e-5


def _coordinate_model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAerospace.Tests.Coordinates.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, shape: tuple[int, ...]) -> np.ndarray:
    if len(shape) == 1:
        return np.array([result[f"{name}[{index}]"][-1] for index in range(1, shape[0] + 1)])
    return np.array(
        [
            [result[f"{name}[{row},{column}]"][-1] for column in range(1, shape[1] + 1)]
            for row in range(1, shape[0] + 1)
        ]
    )


def _geodetic_to_ecef(latitude: float, longitude: float, altitude: float) -> np.ndarray:
    radius = WGS84_A / math.sqrt(1 - WGS84_E2 * math.sin(latitude) ** 2)
    return np.array(
        [
            (radius + altitude) * math.cos(latitude) * math.cos(longitude),
            (radius + altitude) * math.cos(latitude) * math.sin(longitude),
            (radius * (1 - WGS84_E2) + altitude) * math.sin(latitude),
        ]
    )


def _ecef_to_eci_matrix(time: float) -> np.ndarray:
    angle = EARTH_RATE * time
    return np.array(
        [
            [math.cos(angle), -math.sin(angle), 0],
            [math.sin(angle), math.cos(angle), 0],
            [0, 0, 1],
        ]
    )


def _angle_difference(left: float, right: float) -> float:
    return math.atan2(math.sin(left - right), math.cos(left - right))


@pytest.mark.parametrize(
    ("class_name", "latitude", "longitude", "altitude", "round_trip_longitude"),
    [
        ("EquatorPrime", 0.0, 0.0, 0.0, 0.0),
        ("EquatorEast", 0.0, math.pi / 2, 0.0, math.pi / 2),
        ("NorthPole", math.pi / 2, 1.2, 0.0, 0.0),
        ("SouthPole", -math.pi / 2, -2.0, 0.0, 0.0),
        ("GenericPosition", 0.7, -2.4, 1234.0, -2.4),
        ("DatelinePosition", 0.2, math.pi - 1e-9, 35000.0, math.pi - 1e-9),
    ],
)
def test_wgs84_round_trips_and_reference_vectors(
    modelica_backend: str,
    class_name: str,
    latitude: float,
    longitude: float,
    altitude: float,
    round_trip_longitude: float,
):
    result = _coordinate_model(modelica_backend, class_name).simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    ecef = _array(result, "ecef", (3,))
    round_trip = _array(result, "roundTrip", (3,))
    expected_ecef = _geodetic_to_ecef(latitude, longitude, altitude)

    np.testing.assert_allclose(ecef, expected_ecef, atol=1e-6, rtol=0)
    assert round_trip[0] == pytest.approx(latitude, abs=2e-11)
    assert _angle_difference(round_trip[1], round_trip_longitude) == pytest.approx(0, abs=2e-11)
    assert round_trip[2] == pytest.approx(altitude, abs=2e-5)
    np.testing.assert_allclose(_array(result, "blockECEF", (3,)), ecef, atol=1e-10, rtol=0)
    np.testing.assert_allclose(
        _array(result, "blockGeodetic", (3,)),
        round_trip,
        atol=1e-10,
        rtol=0,
    )


def test_ned_frames_local_position_and_blocks(modelica_backend: str):
    result = _coordinate_model(modelica_backend, "GenericPosition").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    ned_to_ecef = _array(result, "nedToECEF", (3, 3))
    ecef_to_ned = _array(result, "ecefToNED", (3, 3))
    np.testing.assert_allclose(ecef_to_ned, ned_to_ecef.T, atol=1e-12, rtol=0)
    np.testing.assert_allclose(ned_to_ecef.T @ ned_to_ecef, np.eye(3), atol=1e-12, rtol=0)
    np.testing.assert_allclose(
        _array(result, "nedRoundTrip", (3,)),
        [120, -35, 8],
        atol=1e-9,
        rtol=0,
    )
    np.testing.assert_allclose(
        _array(result, "localNEDRoundTrip", (3,)),
        [100, 20, -5],
        atol=2e-6,
        rtol=0,
    )
    np.testing.assert_allclose(
        _array(result, "blockNEDRoundTrip", (3,)),
        [120, -35, 8],
        atol=1e-9,
        rtol=0,
    )


def test_rotating_earth_position_velocity_and_acceleration(modelica_backend: str):
    result = _coordinate_model(modelica_backend, "GenericPosition").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    ecef = _array(result, "ecef", (3,))
    eci = _array(result, "eciPosition", (3,))
    eci_velocity = _array(result, "eciVelocity", (3,))
    eci_acceleration = _array(result, "eciAcceleration", (3,))
    rotation = _ecef_to_eci_matrix(4321)
    earth_rate = np.array([0, 0, EARTH_RATE])
    ecef_velocity = np.array([120, -45, 8])
    ecef_acceleration = np.array([1.2, -0.5, 0.25])
    np.testing.assert_allclose(eci, rotation @ ecef, atol=1e-6, rtol=0)
    np.testing.assert_allclose(
        eci_velocity,
        rotation @ (ecef_velocity + np.cross(earth_rate, ecef)),
        atol=1e-9,
        rtol=0,
    )
    np.testing.assert_allclose(
        eci_acceleration,
        rotation
        @ (
            ecef_acceleration
            + 2 * np.cross(earth_rate, ecef_velocity)
            + np.cross(earth_rate, np.cross(earth_rate, ecef))
        ),
        atol=1e-9,
        rtol=0,
    )
    np.testing.assert_allclose(
        _array(result, "ecefPositionRoundTrip", (3,)),
        ecef,
        atol=1e-6,
        rtol=0,
    )
    np.testing.assert_allclose(
        _array(result, "ecefVelocityRoundTrip", (3,)),
        [120, -45, 8],
        atol=1e-9,
        rtol=0,
    )
    np.testing.assert_allclose(
        _array(result, "ecefAccelerationRoundTrip", (3,)),
        [1.2, -0.5, 0.25],
        atol=1e-9,
        rtol=0,
    )


def test_earth_center_is_rejected():
    source = (PACKAGE_ROOT / "Coordinates" / "ecefToGeodetic.mo").read_text(encoding="utf-8")
    assert "Geodetic position is undefined at the Earth center" in source
