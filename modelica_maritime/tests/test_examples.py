from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def test_surface_maneuvering_example_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Examples.SurfaceManeuvering",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=60, step_size=0.1),
        backend=modelica_backend,
    )

    north = np.asarray(result["poseNED[1]"])
    east = np.asarray(result["poseNED[2]"])
    heading = np.asarray(result["poseNED[3]"])
    surge = np.asarray(result["velocityBody[1]"])
    sway = np.asarray(result["velocityBody[2]"])
    yaw_rate = np.asarray(result["velocityBody[3]"])
    dissipation = np.asarray(result["dissipationPower"])

    assert north[-1] > 50
    assert abs(east[-1]) > 3
    assert abs(heading[-1]) > 0.05
    assert np.max(np.abs(surge)) < 10
    assert np.max(np.abs(sway)) < 5
    assert np.max(np.abs(yaw_rate)) < 0.4
    assert np.all(dissipation >= -1e-8)


def test_mmg_turning_circle_example_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Examples.MMGTurningCircle",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=240, step_size=0.2),
        backend=modelica_backend,
    )

    north = np.asarray(result["poseNED[1]"])
    east = np.asarray(result["poseNED[2]"])
    heading = np.asarray(result["poseNED[3]"])
    surge = np.asarray(result["velocityBody[1]"])
    sway = np.asarray(result["velocityBody[2]"])
    yaw_rate = np.asarray(result["velocityBody[3]"])

    assert np.max(heading) > math.pi
    assert np.ptp(north) > 20
    assert np.ptp(east) > 20
    assert np.all(np.isfinite(surge))
    assert np.max(np.abs(surge)) < 10
    assert np.max(np.abs(sway)) < 5
    assert np.max(np.abs(yaw_rate)) < 1


def test_underwater_free_decay_example_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Examples.UnderwaterFreeDecay",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=40, step_size=0.05),
        backend=modelica_backend,
    )

    quaternion_norm = np.asarray(result["quaternionNorm"])
    roll_component = np.asarray(result["quaternion[2]"])
    pitch_component = np.asarray(result["quaternion[3]"])
    relative_surge = np.asarray(result["velocityBody[1]"]) - 0.2
    dissipation = np.asarray(result["dissipationPower"])

    assert np.max(np.abs(quaternion_norm - 1)) < 2e-6
    assert abs(roll_component[-1]) < abs(roll_component[0])
    assert abs(pitch_component[-1]) < abs(pitch_component[0])
    assert abs(relative_surge[-1]) < abs(relative_surge[0])
    assert np.all(dissipation >= -1e-7)


def test_surface_wave_response_example_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Examples.SurfaceWaveResponse",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=60, step_size=0.05),
        backend=modelica_backend,
    )

    position = np.column_stack(
        [np.asarray(result[f"positionNED[{index}]"]) for index in range(1, 4)]
    )
    velocity = np.column_stack(
        [np.asarray(result[f"velocityBody[{index}]"]) for index in range(1, 7)]
    )
    wave_elevation = np.asarray(result["waveElevation"])
    quaternion_norm = np.asarray(result["quaternionNorm"])
    assert position[-1, 0] > 50
    assert np.ptp(wave_elevation) > 0.8
    assert np.max(np.abs(quaternion_norm - 1)) < 5e-6
    assert np.all(np.isfinite(position))
    assert np.max(np.abs(velocity[:, :3])) < 8
    assert np.max(np.abs(velocity[:, 3:])) < 2


def test_underwater_depth_heading_hold_example_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Examples.UnderwaterDepthHeadingHold",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=80, step_size=0.1),
        backend=modelica_backend,
    )

    velocity = np.column_stack(
        [np.asarray(result[f"velocityBody[{index}]"]) for index in range(1, 7)]
    )
    quaternion_norm = np.asarray(result["quaternionNorm"])
    assert abs(result["depthError"][-1]) < 1
    assert abs(result["headingError"][-1]) < 0.1
    assert np.max(np.abs(quaternion_norm - 1)) < 2e-6
    assert np.max(np.abs(velocity[:, :3])) < 6
    assert np.max(np.abs(velocity[:, 3:])) < 1


def test_underwater_waypoint_bathymetry_example_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Examples.UnderwaterWaypointBathymetry",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=100, step_size=0.1),
        backend=modelica_backend,
    )

    quaternion_norm = np.asarray(result["quaternionNorm"])
    assert result["distance"][-1] < 20
    assert abs(result["altitude"][-1] - 15) < 2
    assert abs(result["positionNED[3]"][-1] - result["commandedDepth"][-1]) < 2
    assert np.max(np.abs(quaternion_norm - 1)) < 5e-6


def test_propulsion_failure_response_example_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Examples.PropulsionFailureResponse",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=80, step_size=0.1),
        backend=modelica_backend,
    )

    north = np.asarray(result["poseNED[1]"])
    surge = np.asarray(result["velocityBody[1]"])
    sway = np.asarray(result["velocityBody[2]"])
    yaw_rate = np.asarray(result["velocityBody[3]"])
    assert result["portFailed"][-1] == pytest.approx(1)
    assert north[-1] > 50
    assert np.all(np.isfinite(surge))
    assert np.max(np.abs(surge)) < 6
    assert np.max(np.abs(sway)) < 4
    assert np.max(np.abs(yaw_rate)) < 2
