from __future__ import annotations

import math
import shutil

import numpy as np
import pytest
from polaris import Model, SimulationOptions
from polaris.backends.base import BackendError

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


def test_mmg_component_loads_match_independent_formulas(modelica_backend: str):
    result = _vessel_model(modelica_backend, "MMGComponentValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    relative = np.array([5.0, 1.0, 0.1])
    speed = math.hypot(relative[0], relative[1])
    sway = relative[1] / speed
    yaw_rate = relative[2] * 10 / speed
    scale = 0.5 * 1000 * 10 * 2 * speed**2
    expected_hull = np.array(
        [
            scale * (-0.02 - 0.1 * sway**2),
            scale * (-0.5 * sway + 0.1 * yaw_rate),
            scale * 10 * (0.08 * sway - 0.15 * yaw_rate),
        ]
    )

    axial_velocity = (1 - 0.2) * relative[0]
    advance_ratio = axial_velocity / (2 * 2)
    thrust_coefficient = 0.25 - 0.1 * advance_ratio - 0.05 * advance_ratio**2
    thrust = 1000 * 2**2 * 2**4 * thrust_coefficient
    expected_propeller = np.array([0.9 * thrust, 0, 0])

    rudder_inflow = np.array([1.1 * axial_velocity, 0.8 * (1 - 4 * 0.1)])
    angle_of_attack = 0.15 - math.atan2(rudder_inflow[1], rudder_inflow[0])
    normal_force = (
        0.5 * 1000 * 3 * float(np.dot(rudder_inflow, rudder_inflow)) * 6 * math.sin(angle_of_attack)
    )
    expected_rudder = np.array(
        [
            -0.9 * normal_force * math.sin(0.15),
            -1.2 * normal_force * math.cos(0.15),
            -(-4 + 0.2 * -1) * normal_force * math.cos(0.15),
        ]
    )

    np.testing.assert_allclose(_array(result, "hullLoad", 3), expected_hull, rtol=1e-12)
    np.testing.assert_allclose(
        _array(result, "propellerLoad", 3),
        expected_propeller,
        rtol=1e-12,
    )
    np.testing.assert_allclose(_array(result, "rudderLoad", 3), expected_rudder, rtol=1e-12)
    np.testing.assert_allclose(
        _array(result, "totalLoad", 3),
        expected_hull + expected_propeller + expected_rudder,
        rtol=1e-12,
    )
    np.testing.assert_allclose(_array(result, "componentSumError", 3), 0, atol=1e-10)
    assert result["advanceRatio"][-1] == pytest.approx(advance_ratio)
    assert result["thrustCoefficient"][-1] == pytest.approx(thrust_coefficient)
    assert result["rudderAngleOfAttack"][-1] == pytest.approx(angle_of_attack)


def test_mmg_straight_ahead_equilibrium(modelica_backend: str):
    result = _vessel_model(modelica_backend, "MMGStraightEquilibrium").simulate(
        SimulationOptions(stop_time=10, step_size=0.1),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "mmgLoadBody", 3), [0, 0, 0], atol=1e-8)
    np.testing.assert_allclose(_array(result, "accelerationBody", 3), [0, 0, 0], atol=1e-10)
    np.testing.assert_allclose(_array(result, "velocityBody", 3), [5, 0, 0], atol=1e-10)
    np.testing.assert_allclose(_array(result, "poseNED", 3), [50, 0, 0], atol=2e-5)


def test_mmg_zig_zag_reverses_heading(modelica_backend: str):
    result = _vessel_model(modelica_backend, "MMGZigZag").simulate(
        SimulationOptions(stop_time=120, step_size=0.1),
        backend=modelica_backend,
    )
    heading = np.asarray(result["poseNED[3]"])
    assert np.max(heading) >= math.radians(10)
    assert np.min(heading) <= -math.radians(10)
    assert result["maneuverPhase"][-1] == pytest.approx(3)
    assert np.all(np.isfinite(_array(result, "velocityBody", 3, final=False)))


def test_mmg_invalid_geometry_fails_explicitly(modelica_backend: str):
    with pytest.raises((BackendError, ValueError)):
        _vessel_model(modelica_backend, "MMGInvalidGeometry").simulate(
            SimulationOptions(stop_time=0.1, step_size=0.1),
            backend=modelica_backend,
        )
