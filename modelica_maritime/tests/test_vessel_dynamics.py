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


def _matrix6(result, name: str) -> np.ndarray:
    return np.array(
        [[result[f"{name}[{row},{column}]"][-1] for column in range(1, 7)] for row in range(1, 7)]
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


def test_fossen_components_match_independent_references(modelica_backend: str):
    result = _vessel_model(modelica_backend, "FossenComponentValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    mass = 10.0
    center = np.array([0.1, -0.2, 0.3])
    skew = np.array(
        [[0, -center[2], center[1]], [center[2], 0, -center[0]], [-center[1], center[0], 0]]
    )
    inertia_center = np.array([[2, 0.1, 0], [0.1, 3, 0.2], [0, 0.2, 4]])
    inertia_reference = inertia_center - mass * skew @ skew
    expected_mass = np.block([[mass * np.eye(3), -mass * skew], [mass * skew, inertia_reference]])
    velocity = np.array([2, -1, 0.5, 0.1, -0.2, 0.3])
    momentum = expected_mass @ velocity
    skew_linear = np.array(
        [
            [0, -momentum[2], momentum[1]],
            [momentum[2], 0, -momentum[0]],
            [-momentum[1], momentum[0], 0],
        ]
    )
    skew_angular = np.array(
        [
            [0, -momentum[5], momentum[4]],
            [momentum[5], 0, -momentum[3]],
            [-momentum[4], momentum[3], 0],
        ]
    )
    expected_coriolis = np.block([[np.zeros((3, 3)), -skew_linear], [-skew_linear, -skew_angular]])
    expected_damping = -np.diag([10, 20, 30, 40, 50, 60]) @ velocity
    expected_damping -= np.diag([1, 2, 3, 4, 5, 6]) @ (np.abs(velocity) * velocity)
    roll = 0.2
    rotation = np.array(
        [[1, 0, 0], [0, math.cos(roll), -math.sin(roll)], [0, math.sin(roll), math.cos(roll)]]
    )
    weight_body = rotation.T @ np.array([0, 0, 10 * 9.80665])
    buoyancy_body = rotation.T @ np.array([0, 0, -10 * 9.80665])
    expected_restoring = np.concatenate(
        (
            weight_body + buoyancy_body,
            np.cross(center, weight_body) + np.cross(np.array([0, 0, -0.1]), buoyancy_body),
        )
    )

    np.testing.assert_allclose(_matrix(result, "rotation"), rotation, atol=1e-12)
    np.testing.assert_allclose(_matrix6(result, "rigidBodyMass"), expected_mass, atol=1e-12)
    np.testing.assert_allclose(_matrix6(result, "coriolis"), expected_coriolis, atol=1e-12)
    assert result["coriolisPower"][-1] == pytest.approx(0, abs=1e-10)
    np.testing.assert_allclose(_array(result, "damping", 6), expected_damping, atol=1e-12)
    np.testing.assert_allclose(_array(result, "restoring", 6), expected_restoring, atol=1e-10)
    expected_quaternion_rate = 0.5 * np.array(
        [
            -0.09983341664682815 * 0.1,
            0.9950041652780258 * 0.1,
            0.9950041652780258 * -0.2 - 0.09983341664682815 * 0.3,
            0.9950041652780258 * 0.3 + 0.09983341664682815 * -0.2,
        ]
    )
    np.testing.assert_allclose(
        _array(result, "quaternionRate", 4),
        expected_quaternion_rate,
        atol=1e-12,
    )


def test_fossen_neutral_buoyancy_trim(modelica_backend: str):
    result = _vessel_model(modelica_backend, "FossenNeutralTrim").simulate(
        SimulationOptions(stop_time=5, step_size=0.1),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "positionNED", 3), [1, 2, 3], atol=1e-10)
    np.testing.assert_allclose(_array(result, "velocityBody", 6), 0, atol=1e-10)
    np.testing.assert_allclose(_array(result, "restoringLoadBody", 6), 0, atol=1e-10)
    assert result["quaternionNorm"][-1] == pytest.approx(1, abs=1e-10)


def test_fossen_constant_force_matches_analytic_motion(modelica_backend: str):
    result = _vessel_model(modelica_backend, "FossenConstantForce").simulate(
        SimulationOptions(stop_time=2, step_size=0.05),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "accelerationBody", 6), [2, 0, 0, 0, 0, 0])
    np.testing.assert_allclose(_array(result, "velocityBody", 6), [4, 0, 0, 0, 0, 0], atol=2e-6)
    np.testing.assert_allclose(_array(result, "positionNED", 3), [4, 0, 0], atol=2e-5)


def test_fossen_constant_principal_moment(modelica_backend: str):
    result = _vessel_model(modelica_backend, "FossenConstantMoment").simulate(
        SimulationOptions(stop_time=2, step_size=0.05),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "accelerationBody", 6), [0, 0, 0, 1, 0, 0])
    np.testing.assert_allclose(_array(result, "velocityBody", 6), [0, 0, 0, 2, 0, 0], atol=2e-6)
    expected_quaternion = np.array([math.cos(1), math.sin(1), 0, 0])
    quaternion = _array(result, "quaternion", 4)
    assert abs(float(np.dot(quaternion, expected_quaternion))) == pytest.approx(1, abs=2e-6)
    assert result["quaternionNorm"][-1] == pytest.approx(1, abs=2e-7)


def test_fossen_positive_and_negative_buoyancy(modelica_backend: str):
    result = _vessel_model(modelica_backend, "FossenBuoyancy").simulate(
        SimulationOptions(stop_time=1, step_size=0.05),
        backend=modelica_backend,
    )
    expected_acceleration = 0.2 * 9.80665
    np.testing.assert_allclose(
        _array(result, "accelerationDown", 3),
        [-expected_acceleration, 0, expected_acceleration],
        atol=1e-9,
    )
    np.testing.assert_allclose(
        _array(result, "positionDown", 3),
        [-0.5 * expected_acceleration, 0, 0.5 * expected_acceleration],
        atol=2e-5,
    )


def test_fossen_hydrostatic_free_decay_is_damped(modelica_backend: str):
    result = _vessel_model(modelica_backend, "FossenFreeDecay").simulate(
        SimulationOptions(stop_time=15, step_size=0.05),
        backend=modelica_backend,
    )
    scalar = np.asarray(result["quaternion[1]"])
    roll_component = np.asarray(result["quaternion[2]"])
    roll = 2 * np.arctan2(roll_component, scalar)
    assert abs(roll[-1]) < abs(roll[0])
    assert np.min(roll) < 0
    assert np.all(np.asarray(result["dissipationPower"]) >= -1e-8)
    assert np.max(np.abs(np.asarray(result["velocityBody[1]"]))) < 1e-8


def test_fossen_invalid_mass_fails_explicitly(modelica_backend: str):
    with pytest.raises((BackendError, ValueError)):
        _vessel_model(modelica_backend, "FossenInvalidMass").simulate(
            SimulationOptions(stop_time=0.1, step_size=0.1),
            backend=modelica_backend,
        )
