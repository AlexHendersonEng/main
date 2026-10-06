from __future__ import annotations

import math
import shutil

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def _vessel_model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaMaritime.Tests.VesselDynamics.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, width: int, final: bool = True) -> np.ndarray:
    values = [np.asarray(result[f"{name}[{index}]"]) for index in range(1, width + 1)]
    return np.array([value[-1] for value in values]) if final else np.vstack(values).T


def _matrix(result, name: str) -> np.ndarray:
    return np.array(
        [[result[f"{name}[{row},{column}]"][-1] for column in range(1, 4)] for row in range(1, 4)]
    )


def test_planar_hydrodynamic_components(modelica_backend: str):
    result = _vessel_model(modelica_backend, "HydrodynamicsValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    expected_mass = np.array([[100, 0, 0], [0, 100, 50], [0, 50, 105]])
    velocity = np.array([3, -2, 0.4])
    momentum = expected_mass @ velocity
    expected_coriolis = np.array(
        [[0, 0, -momentum[1]], [0, 0, momentum[0]], [momentum[1], -momentum[0], 0]]
    )
    expected_damping = -np.diag([10, 20, 30]) @ np.array([3, -2, 0.5])
    expected_damping -= np.diag([2, 3, 4]) @ np.array([9, -4, 0.25])
    dynamic_pressure = 0.5 * 1000 * 5**2
    expected_coefficients = dynamic_pressure * 20 * np.array([0.1, -0.2, 10 * 0.03])

    np.testing.assert_allclose(_matrix(result, "rigidBodyMass"), expected_mass, atol=1e-12)
    np.testing.assert_allclose(_matrix(result, "coriolis"), expected_coriolis, atol=1e-12)
    assert result["coriolisPower"][-1] == pytest.approx(0, abs=1e-10)
    np.testing.assert_allclose(_array(result, "dampingLoad", 3), expected_damping, atol=1e-12)
    assert result["dampingPower"][-1] == pytest.approx(
        -float(np.dot(np.array([3, -2, 0.5]), expected_damping))
    )
    np.testing.assert_allclose(
        _array(result, "functionCoefficientLoad", 3),
        expected_coefficients,
        atol=1e-9,
    )
    np.testing.assert_allclose(
        _array(result, "blockCoefficientLoad", 3),
        expected_coefficients,
        atol=1e-9,
    )


def test_planar_constant_force_matches_analytic_motion(modelica_backend: str):
    result = _vessel_model(modelica_backend, "PlanarConstantForce").simulate(
        SimulationOptions(stop_time=3, step_size=0.05),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "accelerationBody", 3), [2, 0, 0], atol=1e-9)
    np.testing.assert_allclose(_array(result, "velocityBody", 3), [8, 0, 0], atol=2e-6)
    np.testing.assert_allclose(_array(result, "poseNED", 3), [15, 0, 0], atol=2e-5)


def test_current_relative_equilibrium_advects_with_water(modelica_backend: str):
    result = _vessel_model(modelica_backend, "PlanarCurrentAdvection").simulate(
        SimulationOptions(stop_time=5, step_size=0.1),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "relativeVelocityBody", 3), [0, 0, 0], atol=1e-10)
    np.testing.assert_allclose(_array(result, "velocityBody", 3), [2, 0, 0], atol=1e-10)
    np.testing.assert_allclose(_array(result, "poseNED", 3), [10, 0, 0], atol=2e-5)
    assert result["dissipationPower"][-1] == pytest.approx(0, abs=1e-10)


def test_linear_damping_dissipates_energy(modelica_backend: str):
    result = _vessel_model(modelica_backend, "PlanarDamping").simulate(
        SimulationOptions(stop_time=5, step_size=0.05),
        backend=modelica_backend,
    )
    expected_speed = 4 * math.exp(-0.2 * 5)
    assert result["velocityBody[1]"][-1] == pytest.approx(expected_speed, rel=2e-6)
    energy = np.asarray(result["kineticEnergy"])
    assert np.all(np.diff(energy) <= 1e-7)
    assert np.all(np.asarray(result["dissipationPower"]) >= -1e-10)
    assert energy[-1] < energy[0]


def test_msl_coefficient_table_interpolation():
    if shutil.which("omc") is None:
        pytest.skip("omc not installed")
    result = _vessel_model("openmodelica", "CoefficientTableValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend="openmodelica",
    )
    np.testing.assert_allclose(
        _array(result, "interpolated", 3),
        np.array([-0.1, -0.25, -0.05]) + np.array([1e-8, 2e-8, 3e-8]),
    )
