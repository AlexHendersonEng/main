from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, name: str) -> Model:
    return Model(
        f"ModelicaMaritime.Tests.Subsystems.{name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, width: int = 6) -> np.ndarray:
    return np.array([result[f"{name}[{index}]"][-1] for index in range(1, width + 1)])


def test_propeller_thrusters_shaft_and_energy(modelica_backend: str):
    result = _model(modelica_backend, "PropulsionValidation").simulate(
        SimulationOptions(stop_time=4, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )

    advance_ratio = 0.8 * 4 / (10 * 2)
    coefficient_thrust = 0.3 - 0.1 * advance_ratio + 0.02 * advance_ratio**2
    coefficient_torque = 0.04 - 0.01 * advance_ratio + 0.005 * advance_ratio**2
    thrust = 0.9 * 1025 * 2**4 * coefficient_thrust * 10**2
    torque = 1025 * 2**5 * coefficient_torque * 10**2

    assert result["advanceRatio"][-1] == pytest.approx(advance_ratio)
    assert result["propellerThrust"][-1] == pytest.approx(thrust)
    assert result["propellerTorque"][-1] == pytest.approx(torque)
    assert result["shaftPower"][-1] == pytest.approx(2 * math.pi * 10 * torque)
    assert result["reverseThrust"][-1] == pytest.approx(-thrust)
    assert result["reverseTorque"][-1] == pytest.approx(-torque)
    assert result["reversePower"][-1] == pytest.approx(2 * math.pi * 10 * torque)
    np.testing.assert_allclose(_array(result, "fixedLoad"), [500, 0, 0, 0, 0, -1000])
    np.testing.assert_allclose(_array(result, "azimuthLoad"), [0, -200, 0, 0, 0, 200], atol=1e-10)
    np.testing.assert_allclose(_array(result, "failedLoad"), np.zeros(6))
    assert result["shaftRate"][-1] == pytest.approx(16 * (1 - math.exp(-(4 - 1) / 0.5)), abs=2e-5)
    assert result["energy"][-1] == pytest.approx(400, abs=2e-5)
    assert result["stateOfCharge"][-1] == pytest.approx(0.4, abs=2e-8)
    assert result["emptyEnergy"][-1] == pytest.approx(0, abs=2e-5)
    assert result["emptyPower"][-1] == pytest.approx(0, abs=2e-5)
    assert result["fullEnergy"][-1] == pytest.approx(1000, abs=2e-5)
    assert result["fullPower"][-1] == pytest.approx(0, abs=2e-5)


def test_actuator_limits_surfaces_ballast_and_buoyancy(modelica_backend: str):
    result = _model(modelica_backend, "ActuatorValidation").simulate(
        SimulationOptions(stop_time=7, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )

    time = np.asarray(result["time"])
    position = np.asarray(result["servoPosition"])
    rate = np.asarray(result["servoRate"])
    assert position[np.argmin(np.abs(time - 0.8))] == pytest.approx(0, abs=1e-7)
    assert position[np.argmin(np.abs(time - 3.2))] == pytest.approx(1, abs=3e-3)
    assert position[-1] == pytest.approx(-0.25, abs=3e-3)
    assert np.max(position) <= 1 + 2e-4
    assert np.min(position) >= -1 - 2e-4
    assert np.max(np.abs(rate)) <= 0.5 + 2e-4

    rudder_lift = 0.5 * 1025 * 2 * 4**2 * 5 * 0.1
    rudder_drag = 0.5 * 1025 * 2 * 4**2 * 0.02
    np.testing.assert_allclose(
        _array(result, "rudderLoad"),
        [-rudder_drag, rudder_lift, 0, 0, 0, -4 * rudder_lift],
    )
    plane_lift = 0.5 * 1025 * 1.5 * 3**2 * 4 * -0.08
    plane_drag = 0.5 * 1025 * 1.5 * 3**2 * 0.01
    np.testing.assert_allclose(
        _array(result, "hydroplaneLoad"),
        [-plane_drag, 0, plane_lift, 0, -2 * plane_lift, 0],
    )
    assert result["ballastMass"][-1] == pytest.approx(1, abs=2e-5)
    assert _array(result, "ballastLoad")[2] == pytest.approx(9.80665, abs=2e-4)
    assert result["displacedVolume"][-1] == pytest.approx(0.8, abs=2e-5)
    assert _array(result, "buoyancyLoad")[2] == pytest.approx(-1025 * 9.80665 * 0.3, abs=2e-3)


def test_propulsor_load_integrates_with_planar_and_six_dof_dynamics(
    modelica_backend: str,
):
    result = _model(modelica_backend, "LoadIntegrationValidation").simulate(
        SimulationOptions(stop_time=2, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )

    assert result["planarVelocity"][-1] == pytest.approx(6, abs=2e-5)
    assert result["mmgVelocity"][-1] == pytest.approx(6, abs=2e-5)
    assert result["spatialVelocity"][-1] == pytest.approx(6, abs=2e-5)
    assert result["planarPosition"][-1] == pytest.approx(6, abs=3e-5)
    assert result["mmgPosition"][-1] == pytest.approx(6, abs=3e-5)
    assert result["spatialPosition"][-1] == pytest.approx(6, abs=3e-5)
