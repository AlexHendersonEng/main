from __future__ import annotations

import math

import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAutomotive.Tests.Longitudinal.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def test_constant_force_matches_analytic_motion(modelica_backend: str):
    result = _model(modelica_backend, "ConstantForce").simulate(
        SimulationOptions(stop_time=4, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["acceleration"][-1] == pytest.approx(2, abs=1e-10)
    assert result["speed"][-1] == pytest.approx(13, abs=2e-6)
    assert result["position"][-1] == pytest.approx(38, abs=2e-5)


def test_quadratic_drag_coastdown_matches_closed_form(modelica_backend: str):
    result = _model(modelica_backend, "Coastdown").simulate(
        SimulationOptions(stop_time=10, step_size=0.05, tolerance=1e-9),
        backend=modelica_backend,
    )
    drag_factor = 0.5 * 1.2 * 0.8 / 1200
    denominator = 1 + drag_factor * 30 * 10
    expected_speed = 30 / denominator
    expected_position = math.log(denominator) / drag_factor
    assert result["speed"][-1] == pytest.approx(expected_speed, abs=2e-6)
    assert result["position"][-1] == pytest.approx(expected_position, abs=2e-5)
    assert result["aerodynamicForce"][-1] == pytest.approx(
        0.5 * 1.2 * 0.8 * expected_speed**2,
        abs=2e-5,
    )


def test_grade_equilibrium_preserves_speed(modelica_backend: str):
    result = _model(modelica_backend, "GradeEquilibrium").simulate(
        SimulationOptions(stop_time=5, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["acceleration"][-1] == pytest.approx(0, abs=2e-10)
    assert result["netForce"][-1] == pytest.approx(0, abs=2e-7)


def test_zero_speed_rolling_resistance_is_zero(modelica_backend: str):
    result = _model(modelica_backend, "ZeroSpeedResistance").simulate(
        SimulationOptions(stop_time=1, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["rollingResistanceForce"][-1] == pytest.approx(0, abs=1e-12)
    assert result["acceleration"][-1] == pytest.approx(0, abs=1e-12)


def test_longitudinal_body_declares_invalid_parameter_guards():
    source = (PACKAGE_ROOT / "VehicleDynamics" / "Longitudinal" / "Body.mo").read_text(
        encoding="utf-8"
    )
    for guard in (
        "assert(mass > 0,",
        "assert(dragArea >= 0,",
        "assert(airDensity >= 0,",
        "assert(rollingResistanceCoefficient >= 0,",
        "assert(resistanceRegularization > 0,",
    ):
        assert guard in source
