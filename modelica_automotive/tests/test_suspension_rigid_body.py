from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAutomotive.Tests.SuspensionRigidBody.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, width: int) -> np.ndarray:
    return np.array([result[f"{name}[{index}]"][-1] for index in range(1, width + 1)])


def test_suspension_components_match_force_definitions(modelica_backend: str):
    result = _model(modelica_backend, "ComponentValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["cornerForce"][-1] == pytest.approx(1800, abs=1e-10)
    assert result["stopForce"][-1] == pytest.approx(2000, abs=1e-10)
    np.testing.assert_allclose(_array(result, "barForce", 2), [200, -200], atol=1e-10)
    assert result["contactImpulse"][-1] / 0.1 == pytest.approx(2000, abs=2e-7)
    assert result["separatedImpulse"][-1] == pytest.approx(0, abs=1e-12)


@pytest.mark.parametrize(
    ("class_name", "output_name", "angular_frequency"),
    [
        ("HeaveMode", "heave", math.sqrt(4 * 30000 / 1200)),
        ("RollMode", "roll", math.sqrt(30000 * 1.6**2 / 600)),
        (
            "PitchMode",
            "pitch",
            math.sqrt(30000 * 4 * 1.35**2 / 1800),
        ),
    ],
)
def test_undamped_body_modes_match_linear_natural_frequency(
    modelica_backend: str,
    class_name: str,
    output_name: str,
    angular_frequency: float,
):
    half_period = math.pi / angular_frequency
    result = _model(modelica_backend, class_name).simulate(
        SimulationOptions(stop_time=half_period, step_size=half_period / 100, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert result[output_name][0] == pytest.approx(0.01, abs=1e-12)
    assert result[output_name][-1] == pytest.approx(-0.01, abs=3e-6)


def test_banked_road_settles_to_roll_equilibrium_and_weight_balance(
    modelica_backend: str,
):
    result = _model(modelica_backend, "BankEquilibrium").simulate(
        SimulationOptions(stop_time=8, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert result["heave"][-1] == pytest.approx(0, abs=2e-5)
    assert result["roll"][-1] == pytest.approx(math.tan(0.04), abs=2e-5)
    assert result["pitch"][-1] == pytest.approx(0, abs=2e-5)
    assert result["forceSum"][-1] == pytest.approx(1200 * 9.80665, abs=2)


def test_full_body_trim_preserves_straight_motion(modelica_backend: str):
    result = _model(modelica_backend, "FullBodyTrim").simulate(
        SimulationOptions(stop_time=4, step_size=0.05, tolerance=1e-9),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "position", 3), [40, 0, 0], atol=2e-5)
    np.testing.assert_allclose(_array(result, "velocityBody", 3), [10, 0, 0], atol=2e-6)
    np.testing.assert_allclose(_array(result, "quaternion", 4), [1, 0, 0, 0], atol=2e-8)
    assert result["quaternionNorm"][-1] == pytest.approx(1, abs=2e-8)


def test_full_body_constant_force_matches_analytic_motion(modelica_backend: str):
    result = _model(modelica_backend, "FullBodyConstantForce").simulate(
        SimulationOptions(stop_time=3, step_size=0.05, tolerance=1e-9),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "accelerationBody", 3), [2, 0, 0], atol=1e-9)
    np.testing.assert_allclose(_array(result, "velocityBody", 3), [16, 0, 0], atol=2e-6)
    np.testing.assert_allclose(_array(result, "position", 3), [39, 0, 0], atol=2e-5)


def test_full_body_constant_moment_matches_principal_axis_solution(
    modelica_backend: str,
):
    result = _model(modelica_backend, "FullBodyConstantMoment").simulate(
        SimulationOptions(stop_time=2, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    np.testing.assert_allclose(_array(result, "angularAcceleration", 3), [1, 0, 0], atol=1e-8)
    np.testing.assert_allclose(_array(result, "angularVelocity", 3), [2, 0, 0], atol=2e-6)
    expected_quaternion = np.array([math.cos(1), math.sin(1), 0, 0])
    quaternion = _array(result, "quaternion", 4)
    assert abs(float(np.dot(quaternion, expected_quaternion))) == pytest.approx(1, abs=3e-6)
    assert result["quaternionNorm"][-1] == pytest.approx(1, abs=3e-7)


def test_full_body_matches_planar_straight_line_limit(modelica_backend: str):
    result = _model(modelica_backend, "PlanarLimit").simulate(
        SimulationOptions(stop_time=4, step_size=0.05, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert result["positionDifference"][-1] == pytest.approx(0, abs=3e-5)
    assert result["velocityDifference"][-1] == pytest.approx(0, abs=3e-6)


def test_suspension_and_full_body_declare_parameter_guards():
    sources = [
        PACKAGE_ROOT / "Suspension" / "CornerSpringDamper.mo",
        PACKAGE_ROOT / "Suspension" / "VerticalTire.mo",
        PACKAGE_ROOT / "Suspension" / "HeaveRollPitchBody.mo",
        PACKAGE_ROOT / "VehicleDynamics" / "RigidBody" / "FullBody.mo",
    ]
    text = "\n".join(path.read_text(encoding="utf-8") for path in sources)
    for guard in (
        "assert(stiffness >= 0,",
        "assert(stiffness > 0,",
        "assert(mass > 0,",
        "assert(massProperties.mass > 0,",
        "assert(quaternionStabilization >= 0,",
    ):
        assert guard in text
