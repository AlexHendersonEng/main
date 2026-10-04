from __future__ import annotations

import math
from pathlib import Path

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_FILE, PACKAGE_ROOT, PACKAGE_SOURCES, PROJECT_ROOT

pytestmark = pytest.mark.integration

ATTITUDE_VALIDATION = PROJECT_ROOT / "tests" / "modelica" / "AttitudeValidation.mo"


def _attitude_model(backend: str, class_name: str = "AttitudeValidation") -> Model:
    files: tuple[Path, ...] = (
        (ATTITUDE_VALIDATION, PACKAGE_ROOT.parent, *PACKAGE_SOURCES)
        if backend == "rumoca"
        else (PACKAGE_FILE, ATTITUDE_VALIDATION)
    )
    return Model(class_name, files=files, libraries=("Modelica",))


def _quaternion(roll: float, pitch: float, yaw: float) -> np.ndarray:
    cr, sr = math.cos(roll / 2), math.sin(roll / 2)
    cp, sp = math.cos(pitch / 2), math.sin(pitch / 2)
    cy, sy = math.cos(yaw / 2), math.sin(yaw / 2)
    return np.array(
        [
            cr * cp * cy + sr * sp * sy,
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
        ]
    )


def _dcm(quaternion: np.ndarray) -> np.ndarray:
    w, x, y, z = quaternion / np.linalg.norm(quaternion)
    return np.array(
        [
            [1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y)],
            [2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x)],
            [2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y)],
        ]
    )


def _hamilton(left: np.ndarray, right: np.ndarray) -> np.ndarray:
    lw, lx, ly, lz = left
    rw, rx, ry, rz = right
    return np.array(
        [
            lw * rw - lx * rx - ly * ry - lz * rz,
            lw * rx + lx * rw + ly * rz - lz * ry,
            lw * ry - lx * rz + ly * rw + lz * rx,
            lw * rz + lx * ry - ly * rx + lz * rw,
        ]
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


@pytest.mark.parametrize(
    ("class_name", "roll", "pitch", "yaw"),
    [
        ("AttitudeIdentity", 0.0, 0.0, 0.0),
        ("AttitudePrincipalAxes", math.pi / 2, 0.0, -math.pi / 3),
        ("AttitudeNearSingular", 0.3, math.pi / 2 - 1e-7, -0.8),
        ("AttitudeWideYaw", -0.4, 0.25, 2.6),
    ],
)
def test_attitude_round_trips_and_reference_values(
    modelica_backend: str,
    class_name: str,
    roll: float,
    pitch: float,
    yaw: float,
):
    rates = np.array([0.1, -0.2, 0.3])
    result = _attitude_model(modelica_backend, class_name).simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    expected_quaternion = _quaternion(roll, pitch, yaw)
    expected_dcm = _dcm(expected_quaternion)
    quaternion = _array(result, "quaternion", (4,))
    dcm = _array(result, "dcm", (3, 3))
    euler = _array(result, "eulerRoundTrip", (3,))
    round_trip = _array(result, "quaternionRoundTrip", (4,))
    derivative = _array(result, "quaternionDerivative", (4,))

    assert quaternion == pytest.approx(expected_quaternion, abs=1e-10)
    np.testing.assert_allclose(dcm, expected_dcm, atol=1e-10, rtol=0)
    assert euler == pytest.approx([roll, pitch, yaw], abs=2e-7)
    assert abs(float(np.dot(quaternion, round_trip))) == pytest.approx(1, abs=1e-10)
    assert derivative == pytest.approx(
        0.5 * _hamilton(expected_quaternion, np.array([0, *rates])),
        abs=1e-10,
    )
    np.testing.assert_allclose(dcm @ dcm.T, np.eye(3), atol=1e-10, rtol=0)
    assert np.linalg.det(dcm) == pytest.approx(1, abs=1e-10)
    assert result["rotationValid"][-1] == pytest.approx(1)
    assert result["invalidRotationValid"][-1] == pytest.approx(0)
    assert result["determinant"][-1] == pytest.approx(1, abs=1e-10)
    assert _array(result, "conjugateProduct", (4,)) == pytest.approx([1, 0, 0, 0], abs=1e-10)
    assert _array(result, "rotatedVector", (3,)) == pytest.approx(
        expected_dcm @ np.array([1, 0, 0]), abs=1e-10
    )

    assert _array(result, "blockQuaternion", (4,)) == pytest.approx(quaternion, abs=1e-12)
    assert _array(result, "blockEuler", (3,)) == pytest.approx(euler, abs=1e-12)
    np.testing.assert_allclose(_array(result, "blockDCM", (3, 3)), dcm, atol=1e-12, rtol=0)
    assert _array(result, "blockRotatedVector", (3,)) == pytest.approx(
        _array(result, "rotatedVector", (3,)), abs=1e-12
    )


def test_vector_matrix_helpers_and_angle_wrapping(modelica_backend: str):
    result = _attitude_model(modelica_backend).simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    assert _array(result, "crossResult", (3,)) == pytest.approx([0, 0, 1])
    assert _array(result, "normalizedVector", (3,)) == pytest.approx([0.6, 0.8, 0])
    np.testing.assert_allclose(
        _array(result, "skew", (3, 3)),
        [[0, -3, 2], [3, 0, -1], [-2, 1, 0]],
        atol=1e-12,
        rtol=0,
    )
    assert result["wrappedAngle"][-1] == pytest.approx(0.71)


def test_normalization_functions_reject_near_zero_inputs():
    vector_source = (PACKAGE_ROOT / "Mathematics" / "normalizeVector3.mo").read_text(
        encoding="utf-8"
    )
    quaternion_source = (PACKAGE_ROOT / "Mathematics" / "normalizeQuaternion.mo").read_text(
        encoding="utf-8"
    )

    assert "Cannot normalize a near-zero vector" in vector_source
    assert "Cannot normalize a near-zero quaternion" in quaternion_source
    assert "Constants.Numerics.small" in vector_source
    assert "Constants.Numerics.small" in quaternion_source
    dcm_source = (PACKAGE_ROOT / "Mathematics" / "dcmToQuaternion.mo").read_text(encoding="utf-8")
    assert "DCM must be an orthogonal proper-rotation matrix" in dcm_source
