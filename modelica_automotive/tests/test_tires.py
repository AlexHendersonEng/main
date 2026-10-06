from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration


def _evaluate(backend: str):
    model = Model(
        "ModelicaAutomotive.Tests.Tires.TireEvaluation",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )
    return model.simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=backend,
    )


def _value(result, *names: str) -> float:
    for name in names:
        if name in result.variables:
            return float(result[name][-1])
    raise KeyError(f"none of the result variables are available: {names}")


def _force(result, output_name: str, component_name: str) -> np.ndarray:
    return np.array(
        [
            _value(
                result,
                f"{output_name}[1]",
                f"{component_name}.longitudinalForce",
            ),
            _value(
                result,
                f"{output_name}[2]",
                f"{component_name}.lateralForce",
            ),
        ]
    )


def _combined_limit(force: np.ndarray, limit: float) -> np.ndarray:
    magnitude = np.linalg.norm(force)
    return force * min(1.0, limit / magnitude) if magnitude > 0 else force


def _fiala_component(slip: float, stiffness: float, limit: float) -> float:
    if limit <= 1e-9:
        return 0
    critical = 3 * limit / stiffness
    if abs(slip) < critical:
        return (
            stiffness * slip
            - stiffness**2 * abs(slip) * slip / (3 * limit)
            + stiffness**3 * slip**3 / (27 * limit**2)
        )
    return math.copysign(limit, slip)


def _magic_component(
    slip: float,
    stiffness: float,
    shape: float,
    curvature: float,
    peak: float,
) -> float:
    if peak <= 1e-9:
        return 0
    stiffness_factor = stiffness / (shape * peak)
    argument = stiffness_factor * slip
    return peak * math.sin(
        shape * math.atan(argument - curvature * (argument - math.atan(argument)))
    )


def test_tire_kinematics_match_regularized_definitions(modelica_backend: str):
    result = _evaluate(modelica_backend)
    reference_speed = math.sqrt(20**2 + 0.1**2)
    assert _value(result, "calculatedSlipRatio", "kinematics.slipRatio") == pytest.approx(
        (0.3 * 70 - 20) / reference_speed,
        abs=1e-12,
    )
    expected_angle = math.atan2(1, reference_speed)
    actual_angle = (
        _value(result, "calculatedSlipAngle", "kinematics.slipAngle")
        if any(name in result.variables for name in ("calculatedSlipAngle", "kinematics.slipAngle"))
        else _value(result, "integratedSlipAngle") / 0.1
    )
    assert actual_angle == pytest.approx(expected_angle, abs=2e-10)


def test_tire_models_match_independent_combined_slip_references(
    modelica_backend: str,
):
    slip_ratio = 0.08
    slip_angle = 0.06
    normal_load = 4000
    friction = 0.9
    force_limit = normal_load * friction
    result = _evaluate(modelica_backend)

    expected_linear = _combined_limit(
        np.array([80000 * slip_ratio, 60000 * slip_angle]),
        force_limit,
    )
    fiala_pure = np.array(
        [
            _fiala_component(slip_ratio, 100000, force_limit),
            _fiala_component(math.tan(slip_angle), 80000, force_limit),
        ]
    )
    expected_fiala = _combined_limit(fiala_pure, force_limit)
    magic_pure = np.array(
        [
            _magic_component(slip_ratio, 90000, 1.65, 0.2, force_limit),
            _magic_component(math.tan(slip_angle), 70000, 1.3, -1.6, force_limit),
        ]
    )
    expected_magic = _combined_limit(magic_pure, force_limit)

    np.testing.assert_allclose(
        _force(result, "linearForce", "linear"),
        expected_linear,
        atol=1e-8,
    )
    np.testing.assert_allclose(
        _force(result, "fialaForce", "fiala"),
        expected_fiala,
        atol=1e-8,
    )
    np.testing.assert_allclose(
        _force(result, "magicFormulaForce", "magicFormula"),
        expected_magic,
        atol=1e-8,
    )
    utilizations = (
        _value(result, "utilization[1]", "linear.utilization"),
        _value(result, "utilization[2]", "fiala.utilization"),
        _value(result, "utilization[3]", "magicFormula.utilization"),
    )
    assert max(utilizations) <= 1 + 1e-12


def test_saturated_fiala_force_scales_with_normal_load(modelica_backend: str):
    model = Model(
        "ModelicaAutomotive.Tests.Tires.TireLoadValidation",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    low_force = result["lowLoadImpulse"][-1] / 0.1
    high_force = result["highLoadImpulse"][-1] / 0.1
    assert low_force == pytest.approx(1600, abs=2e-6)
    assert high_force == pytest.approx(3200, abs=2e-6)
    assert high_force / low_force == pytest.approx(2, abs=2e-9)


def test_tire_models_declare_parameter_and_regularization_guards():
    sources = [
        PACKAGE_ROOT / "Tires" / "Kinematics.mo",
        PACKAGE_ROOT / "Tires" / "CombinedSlipLimiter.mo",
        PACKAGE_ROOT / "Tires" / "LinearTire.mo",
        PACKAGE_ROOT / "Tires" / "FialaTire.mo",
        PACKAGE_ROOT / "Tires" / "MagicFormulaTire.mo",
    ]
    text = "\n".join(path.read_text(encoding="utf-8") for path in sources)
    for guard in (
        "assert(rollingRadius > 0,",
        "assert(velocityRegularization > 0,",
        "assert(frictionCoefficient >= 0,",
        "assert(parameters.longitudinalStiffness",
    ):
        assert guard in text
