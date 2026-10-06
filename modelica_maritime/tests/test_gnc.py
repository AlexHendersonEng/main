from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, name: str) -> Model:
    return Model(
        f"ModelicaMaritime.Tests.GNC.{name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, width: int = 3, final: bool = True) -> np.ndarray:
    values = [np.asarray(result[f"{name}[{index}]"]) for index in range(1, width + 1)]
    return np.array([value[-1] for value in values]) if final else np.vstack(values).T


def test_ideal_and_nonideal_maritime_sensors_are_deterministic(
    modelica_backend: str,
):
    options = SimulationOptions(stop_time=2, step_size=0.05)
    first = _model(modelica_backend, "SensorValidation").simulate(options, backend=modelica_backend)
    second = _model(modelica_backend, "SensorValidation").simulate(
        options, backend=modelica_backend
    )

    assert first["idealScalarMeasurement"][-1] == pytest.approx(4)
    np.testing.assert_allclose(_array(first, "idealVectorMeasurement"), [1, 2, 3])
    np.testing.assert_allclose(_array(first, "measuredPosition"), [11, 22, 33])
    np.testing.assert_allclose(_array(first, "measuredVelocity"), [1.1, 2.2, 3.3])
    assert first["measuredHeading"][-1] == pytest.approx((6.25 + 0.1) % (2 * math.pi))
    assert first["measuredSpeed"][-1] == pytest.approx(5)
    assert first["measuredSurge"][-1] == pytest.approx(3.2)
    np.testing.assert_allclose(_array(first, "measuredAcceleration"), [1.1, 2.2, 3.3])
    np.testing.assert_allclose(_array(first, "measuredRates"), [0.11, 0.22, 0.33])
    expected_pressure = 101325 + 1025 * 9.80665 * 20 + 100
    assert first["measuredPressure"][-1] == pytest.approx(expected_pressure)
    assert first["measuredDepth"][-1] == pytest.approx(20 + 100 / (1025 * 9.80665))
    assert first["measuredAltitude"][-1] == pytest.approx(70.5)
    assert first["measuredRange"][-1] == pytest.approx(6)
    assert first["measuredBearing"][-1] == pytest.approx(math.atan2(4, 3) + 0.1)

    first_scalar = np.asarray(first["scalarMeasurement"])
    second_scalar = np.asarray(second["scalarMeasurement"])
    first_vector = _array(first, "vectorMeasurement", final=False)
    second_vector = _array(second, "vectorMeasurement", final=False)
    np.testing.assert_allclose(first_scalar, second_scalar, atol=1e-12, rtol=0)
    np.testing.assert_allclose(first_vector, second_vector, atol=1e-12, rtol=0)
    np.testing.assert_allclose(first_scalar / 0.05, np.round(first_scalar / 0.05))
    np.testing.assert_allclose(first_vector / 0.02, np.round(first_vector / 0.02))


def test_navigation_fusion_and_guidance_geometry(modelica_backend: str):
    result = _model(modelica_backend, "NavigationGuidanceValidation").simulate(
        SimulationOptions(stop_time=1, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )

    assert result["speed"][-1] == pytest.approx(math.sqrt(26))
    assert result["horizontalSpeed"][-1] == pytest.approx(5)
    assert result["groundTrack"][-1] == pytest.approx(math.atan2(4, 3))
    assert result["depthRate"][-1] == pytest.approx(1)
    assert result["sideslip"][-1] == pytest.approx(math.atan2(1, 4))
    np.testing.assert_allclose(_array(result, "deadReckonedPosition"), [2, 4, 3.5])
    assert result["deadReckonedHeading"][-1] == pytest.approx((6.3) % (2 * math.pi))

    equilibrium_heading = 2 * math.pi + 0.2 + 0.1 / 2
    expected_heading = equilibrium_heading + (6.2 - equilibrium_heading) * math.exp(-2)
    assert result["filteredHeading"][-1] == pytest.approx(
        expected_heading % (2 * math.pi), abs=2e-6
    )
    expected_position = np.array([11, 5, 2]) * (1 - math.exp(-1))
    np.testing.assert_allclose(_array(result, "fusedPosition"), expected_position, atol=2e-6)

    assert result["pathHeading"][-1] == pytest.approx(0)
    assert result["crossTrackError"][-1] == pytest.approx(10)
    assert result["alongTrackDistance"][-1] == pytest.approx(20)
    assert result["commandedLineHeading"][-1] == pytest.approx(
        (-math.atan2(10, 20)) % (2 * math.pi)
    )
    assert result["waypointHeading"][-1] == pytest.approx(math.atan2(4, 3))
    assert result["waypointDepth"][-1] == pytest.approx(17)
    assert result["waypointDistance"][-1] == pytest.approx(13)
    assert result["depthCommand"][-1] == pytest.approx(30)
    assert result["altitudeDepthCommand"][-1] == pytest.approx(80)
    assert result["compensatedHeading"][-1] == pytest.approx(math.atan2(-0.5, 2) % (2 * math.pi))
    assert result["requiredWaterSpeed"][-1] == pytest.approx(math.sqrt(4.25))
    assert result["headingError"][-1] == pytest.approx(
        math.atan2(math.sin(0.1 - 6.2), math.cos(0.1 - 6.2))
    )
    assert result["depthError"][-1] == pytest.approx(5)
    assert result["speedError"][-1] == pytest.approx(1)


def test_limited_control_wrap_antiwindup_and_allocation(modelica_backend: str):
    result = _model(modelica_backend, "ControlValidation").simulate(
        SimulationOptions(stop_time=6, step_size=0.01, tolerance=1e-9),
        backend=modelica_backend,
    )

    command = np.asarray(result["command"])
    time = np.asarray(result["time"])
    assert np.max(command) <= 1 + 1e-8
    assert np.min(command) >= -1 - 1e-8
    assert command[np.argmin(np.abs(time - 1))] == pytest.approx(1, abs=1e-5)
    assert np.min(command[time >= 2]) < -0.1
    assert abs(command[-1]) < 5e-3
    assert abs(result["plantMeasurement"][-1]) < 5e-3
    expected_heading_error = math.atan2(math.sin(0.1 - 6.2), math.cos(0.1 - 6.2))
    assert result["headingError"][-1] == pytest.approx(expected_heading_error)
    assert abs(result["headingCommand"][-1]) <= 0.5 + 1e-8
    assert result["portCommand"][-1] == pytest.approx(1)
    assert result["starboardCommand"][-1] == pytest.approx(0.5)
    assert result["rudderCommand"][-1] == pytest.approx(0.9)


def test_surface_los_current_compensated_path_is_bounded(modelica_backend: str):
    result = _model(modelica_backend, "SurfaceClosedLoop").simulate(
        SimulationOptions(stop_time=120, step_size=0.1, tolerance=1e-8),
        backend=modelica_backend,
    )

    north = np.asarray(result["poseNED[1]"])
    east = np.asarray(result["poseNED[2]"])
    surge = np.asarray(result["velocityBody[1]"])
    sway = np.asarray(result["velocityBody[2]"])
    heading_error = np.asarray(result["headingError"])
    assert north[-1] > 150
    assert abs(east[-1]) < 5
    assert abs(result["crossTrackError"][-1]) < 5
    assert abs(heading_error[-1]) < 0.1
    assert np.max(np.abs(surge)) < 5
    assert np.max(np.abs(sway)) < 3


def test_underwater_waypoint_depth_heading_path_is_bounded(modelica_backend: str):
    result = _model(modelica_backend, "UnderwaterClosedLoop").simulate(
        SimulationOptions(stop_time=100, step_size=0.1, tolerance=1e-8),
        backend=modelica_backend,
    )

    position = _array(result, "positionNED", final=False)
    velocity = _array(result, "velocityBody", width=6, final=False)
    quaternion_norm = np.asarray(result["quaternionNorm"])
    assert result["distance"][-1] < 20
    assert abs(result["depthError"][-1]) < 2
    assert abs(result["headingError"][-1]) < 0.15
    assert np.max(np.abs(quaternion_norm - 1)) < 2e-6
    assert np.all(np.isfinite(position))
    assert np.max(np.abs(velocity[:, :3])) < 6
    assert np.max(np.abs(velocity[:, 3:])) < 1
