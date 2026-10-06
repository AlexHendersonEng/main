from __future__ import annotations

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, class_name: str, *, example: bool = True) -> Model:
    namespace = "Examples" if example else "Tests.Scenarios"
    return Model(
        f"ModelicaAutomotive.{namespace}.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def test_scenario_components_produce_standardized_metrics(modelica_backend: str):
    result = _model(modelica_backend, "ScenarioComponents", example=False).simulate(
        SimulationOptions(stop_time=3, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert result["lateralPosition"][-1] == pytest.approx(2)
    heading = (
        result["heading"][-1]
        if "heading" in result.variables
        else result["headingIntegral"][-1] / 3
    )
    assert heading > 0
    assert result["distanceTravelled"][-1] == pytest.approx(6, abs=1e-8)
    assert result["absoluteLateralErrorIntegral"][-1] == pytest.approx(1.5, abs=1e-8)
    assert result["phase"][-1] == pytest.approx(2)
    assert result["complete"][-1] == pytest.approx(1)
    assert result["failed"][-1] == pytest.approx(0)


def test_double_lane_change_tracks_and_completes(modelica_backend: str):
    result = _model(modelica_backend, "DoubleLaneChange").simulate(
        SimulationOptions(stop_time=8, step_size=0.01, tolerance=1e-8),
        backend=modelica_backend,
    )
    assert result["completion"][-1] == pytest.approx(1)
    assert np.max(np.abs(result["lateralError"])) < 1
    assert abs(result["lateralError"][-1]) < 0.15
    assert result["failure"][-1] == pytest.approx(0)
    assert result["distanceTravelled"][-1] == pytest.approx(96, abs=1e-7)


def test_split_friction_abs_modulates_and_slows_vehicle(modelica_backend: str):
    result = _model(modelica_backend, "SplitFrictionABS").simulate(
        SimulationOptions(stop_time=4, step_size=0.005, tolerance=1e-8),
        backend=modelica_backend,
    )
    speed = np.asarray(result["speed"])
    left_slip = np.asarray(result["slipRatio[1]"])
    right_slip = np.asarray(result["slipRatio[2]"])
    left_command = np.asarray(result["brakeCommand[1]"])
    right_command = np.asarray(result["brakeCommand[2]"])
    assert speed[-1] < speed[0]
    assert np.min(left_command) < 0.98
    assert np.min(right_command) < 0.98
    assert np.min(left_slip) > -0.5
    assert np.min(right_slip) > -0.5
    assert result["distanceTravelled"][-1] > 10


def test_traction_control_limits_launch_slip(modelica_backend: str):
    result = _model(modelica_backend, "TractionControlledLaunch").simulate(
        SimulationOptions(stop_time=6, step_size=0.005, tolerance=1e-8),
        backend=modelica_backend,
    )
    command = np.asarray(result["propulsionCommand"])
    slip = np.maximum(result["slipRatio[1]"], result["slipRatio[2]"])
    assert result["speed"][-1] > result["speed"][0]
    assert np.min(command) < 0.8
    assert np.max(slip) < 0.35
    assert np.max(result["tireForce[1]"]) > 1000


def test_stability_control_tracks_turn_and_rejects_disturbance(modelica_backend: str):
    result = _model(modelica_backend, "StabilityControlledTurn").simulate(
        SimulationOptions(stop_time=8, step_size=0.01, tolerance=1e-8),
        backend=modelica_backend,
    )
    time = np.asarray(result.time)
    yaw_rate = np.asarray(result["yawRate"])
    during_turn = np.argmin(np.abs(time - 4))
    assert yaw_rate[during_turn] == pytest.approx(0.2, abs=0.04)
    assert abs(yaw_rate[-1]) < 0.03
    assert np.max(np.abs(result["yawMomentCommand"])) <= 3500 + 1e-8
    assert result["position[1]"][-1] > 60


def test_rough_road_ride_is_bounded(modelica_backend: str):
    result = _model(modelica_backend, "RoughRoadRide").simulate(
        SimulationOptions(stop_time=8, step_size=0.005, tolerance=1e-8),
        backend=modelica_backend,
    )
    assert np.max(np.abs(result["heave"])) < 0.03
    assert np.max(np.abs(result["roll"])) < 0.03
    assert np.max(np.abs(result["pitch"])) < 0.02
    for index in range(1, 5):
        assert np.min(result[f"suspensionForce[{index}]"]) > 0


def test_integrated_drive_cycle_tracks_and_recovers_energy(modelica_backend: str):
    result = _model(modelica_backend, "IntegratedDriveCycle").simulate(
        SimulationOptions(stop_time=24, step_size=0.02, tolerance=1e-8),
        backend=modelica_backend,
    )
    time = np.asarray(result.time)
    speed = np.asarray(result["speed"])
    source_power = np.asarray(result["sourcePower"])
    state_of_charge = np.asarray(result["stateOfCharge"])
    cruise = np.argmin(np.abs(time - 11.5))
    low_speed = np.argmin(np.abs(time - 17.5))
    assert speed[cruise] == pytest.approx(18, abs=1)
    assert speed[low_speed] == pytest.approx(8, abs=0.8)
    assert speed[-1] < 1
    assert np.min(source_power) < -10000
    assert np.max(state_of_charge[12 * 50 :]) > state_of_charge[12 * 50]
    assert result["distanceTravelled"][-1] > 150
