from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def _gnc_model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAerospace.Tests.GuidanceNavigationControl.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _at_time(result, name: str, sample_time: float) -> float:
    time = np.asarray(result["time"])
    values = np.asarray(result[name])
    return float(values[np.argmin(np.abs(time - sample_time))])


def test_guidance_geometry_and_navigation_kinematics(modelica_backend: str):
    result = _gnc_model(modelica_backend, "GuidanceNavigationValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["waypointHeading"][-1] == pytest.approx(math.pi / 4)
    assert result["waypointFlightPathAngle"][-1] == pytest.approx(math.atan2(100, math.sqrt(20000)))
    assert result["waypointHorizontalDistance"][-1] == pytest.approx(math.sqrt(20000))
    assert result["waypointDistance"][-1] == pytest.approx(math.sqrt(30000))
    assert result["waypointReached"][-1] == pytest.approx(0)
    assert result["coincidentWaypointReached"][-1] == pytest.approx(1)

    assert result["pathHeading"][-1] == pytest.approx(0)
    assert result["commandedPathHeading"][-1] == pytest.approx(-math.atan2(10, 50))
    assert result["crossTrackError"][-1] == pytest.approx(10)
    assert result["alongTrackDistance"][-1] == pytest.approx(20)
    assert result["headingError"][-1] == pytest.approx(math.radians(2))
    assert result["altitudeError"][-1] == pytest.approx(100)
    assert result["speedError"][-1] == pytest.approx(20)

    expected_ground_speed = math.sqrt(20000)
    assert result["groundSpeed"][-1] == pytest.approx(expected_ground_speed)
    assert result["speed"][-1] == pytest.approx(math.sqrt(20400))
    assert result["groundTrack"][-1] == pytest.approx(math.pi / 4)
    assert result["flightPathAngle"][-1] == pytest.approx(math.atan2(20, expected_ground_speed))


def test_complementary_filter_response_and_angle_wrapping(modelica_backend: str):
    result = _gnc_model(modelica_backend, "ComplementaryFilterValidation").simulate(
        SimulationOptions(stop_time=1, step_size=0.01, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert result["scalarEstimate"][-1] == pytest.approx(10 * (1 - math.exp(-2)), abs=2e-5)
    target_equivalent = -3.1 + 2 * math.pi
    unwrapped_angle = target_equivalent + (3.1 - target_equivalent) * math.exp(-2)
    expected_angle = math.atan2(math.sin(unwrapped_angle), math.cos(unwrapped_angle))
    assert result["angleEstimate"][-1] == pytest.approx(expected_angle, abs=2e-5)


def test_msl_control_wrappers_and_closed_loop(modelica_backend: str):
    if modelica_backend == "rumoca":
        pytest.skip(
            "Rumoca does not yet support the MSL native table constructor used by the gain schedule"
        )
    result = _gnc_model(modelica_backend, "ControlValidation").simulate(
        SimulationOptions(stop_time=6, step_size=0.01, tolerance=1e-8),
        backend=modelica_backend,
    )
    assert _at_time(result, "pidCommand", 1) == pytest.approx(1, abs=2e-3)
    assert _at_time(result, "response", 2.9) > 0.98
    assert result["pidCommand"][-1] == pytest.approx(0, abs=3e-3)
    assert result["response"][-1] == pytest.approx(0, abs=3e-3)
    assert result["controlError"][-1] == pytest.approx(0, abs=3e-3)

    assert _at_time(result, "scheduledGains[1]", 0.5) == pytest.approx(1)
    assert _at_time(result, "scheduledGains[1]", 1.5) == pytest.approx(1.5)
    assert _at_time(result, "scheduledGains[1]", 2.5) == pytest.approx(2)
    assert result["scheduledGains[1]"][-1] == pytest.approx(4)
    assert result["scheduledGains[2]"][-1] == pytest.approx(0.4)
    assert result["scheduledGains[3]"][-1] == pytest.approx(1)
    assert _at_time(result, "selectedCommand", 1) == pytest.approx(-2)
    assert _at_time(result, "selectedCommand", 2) == pytest.approx(2)
    assert _at_time(result, "limitedCommand", 1.5) == pytest.approx(0.5, abs=3e-3)
    assert _at_time(result, "limitedCommand", 3) == pytest.approx(1, abs=2e-3)
    assert _at_time(result, "limitedCommand", 4.5) == pytest.approx(0, abs=3e-3)
    assert result["limitedCommand"][-1] == pytest.approx(-1, abs=2e-3)
