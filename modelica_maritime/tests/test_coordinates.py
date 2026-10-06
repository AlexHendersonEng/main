from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions
from polaris.backends.base import BackendError

from conftest import library_model_files

pytestmark = pytest.mark.integration


def _coordinate_model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaMaritime.Tests.Coordinates.{class_name}",
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


def _rotation_321(roll: float, pitch: float, heading: float) -> np.ndarray:
    c_phi, s_phi = math.cos(roll), math.sin(roll)
    c_theta, s_theta = math.cos(pitch), math.sin(pitch)
    c_psi, s_psi = math.cos(heading), math.sin(heading)
    return np.array(
        [
            [
                c_theta * c_psi,
                s_phi * s_theta * c_psi - c_phi * s_psi,
                c_phi * s_theta * c_psi + s_phi * s_psi,
            ],
            [
                c_theta * s_psi,
                s_phi * s_theta * s_psi + c_phi * c_psi,
                c_phi * s_theta * s_psi - s_phi * c_psi,
            ],
            [-s_theta, s_phi * c_theta, c_phi * c_theta],
        ]
    )


def _euler_rate(roll: float, pitch: float, rates: np.ndarray) -> np.ndarray:
    transform = np.array(
        [
            [1, math.sin(roll) * math.tan(pitch), math.cos(roll) * math.tan(pitch)],
            [0, math.cos(roll), -math.sin(roll)],
            [0, math.sin(roll) / math.cos(pitch), math.cos(roll) / math.cos(pitch)],
        ]
    )
    return transform @ rates


def test_body_ned_rotations_and_kinematics(modelica_backend: str):
    result = _coordinate_model(modelica_backend, "CoordinateValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    roll, pitch, heading = 0.2, -0.3, 0.7
    rotation = _rotation_321(roll, pitch, heading)
    rates = np.array([0.1, 0.2, 0.3])

    np.testing.assert_allclose(_array(result, "rotation", (3, 3)), rotation, atol=1e-12)
    np.testing.assert_allclose(rotation.T @ rotation, np.eye(3), atol=1e-12)
    np.testing.assert_allclose(_array(result, "identityVector", (3,)), [1, 2, 3], atol=1e-12)
    np.testing.assert_allclose(_array(result, "yawVector", (3,)), [0, 1, 0], atol=1e-12)
    np.testing.assert_allclose(_array(result, "roundTrip", (3,)), [2, -1, 0.5], atol=1e-12)
    np.testing.assert_allclose(_array(result, "planarRate", (3,)), [-1, 2, 0.3], atol=1e-12)

    expected_euler_rate = _euler_rate(roll, pitch, rates)
    np.testing.assert_allclose(
        _array(result, "eulerRate", (3,)),
        expected_euler_rate,
        atol=1e-12,
    )
    expected_rigid = np.concatenate((rotation @ np.array([2, -1, 0.5]), expected_euler_rate))
    np.testing.assert_allclose(
        _array(result, "rigidStateRate", (6,)),
        expected_rigid,
        atol=1e-12,
    )
    assert np.all(np.isfinite(_array(result, "nearSingularRate", (3,))))


def test_math_depth_and_altitude_conventions(modelica_backend: str):
    result = _coordinate_model(modelica_backend, "CoordinateValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(
        _array(result, "skew", (3, 3)),
        [[0, -3, 2], [3, 0, -1], [-2, 1, 0]],
        atol=1e-12,
    )
    assert result["wrappedHeading"][-1] == pytest.approx(1.5 * math.pi)
    assert result["depth"][-1] == pytest.approx(30)
    assert result["altitude"][-1] == pytest.approx(70)


def test_euler_rate_singularity_fails_explicitly(modelica_backend: str):
    with pytest.raises((BackendError, ValueError)):
        _coordinate_model(modelica_backend, "EulerSingularity").simulate(
            SimulationOptions(stop_time=0.1, step_size=0.1),
            backend=modelica_backend,
        )
