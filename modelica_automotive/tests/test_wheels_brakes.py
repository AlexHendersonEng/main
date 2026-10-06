from __future__ import annotations

import math

import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAutomotive.Tests.WheelsBrakes.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def test_wheel_constant_torque_matches_analytic_rotation(modelica_backend: str):
    result = _model(modelica_backend, "WheelConstantTorque").simulate(
        SimulationOptions(stop_time=2, step_size=0.05),
        backend=modelica_backend,
    )
    assert result["angularAcceleration"][-1] == pytest.approx(5, abs=1e-10)
    assert result["angularVelocity"][-1] == pytest.approx(15, abs=2e-6)


def test_brake_models_match_static_and_first_order_references(
    modelica_backend: str,
):
    result = _model(modelica_backend, "BrakeEvaluation").simulate(
        SimulationOptions(
            stop_time=1,
            step_size=0.02,
            tolerance=1e-9,
            outputs=(
                "idealTorque",
                "dynamicTorque",
                "application",
                "limitedTorque",
                "limitedMagnitude",
            ),
        ),
        backend=modelica_backend,
    )
    direction = 20 / math.sqrt(20**2 + 0.1**2)
    expected_application = 1 - math.exp(-1 / 0.5)
    assert result["idealTorque"][-1] == pytest.approx(-1800 * direction, abs=1e-8)
    assert result["application"][-1] == pytest.approx(expected_application, abs=2e-6)
    assert result["dynamicTorque"][-1] == pytest.approx(
        -3000 * expected_application * direction,
        abs=2e-3,
    )
    assert result["limitedTorque"][-1] == pytest.approx(-1200 * direction, abs=1e-8)
    assert abs(result["limitedTorque"][-1]) / direction == pytest.approx(
        1200,
        abs=1e-8,
    )


def test_mapped_brake_interpolates_with_msl_table(modelica_backend: str):
    if modelica_backend == "rumoca":
        pytest.skip("Rumoca 0.10 cannot lower the MSL native table constructor")
    result = _model(modelica_backend, "MappedBrakeEvaluation").simulate(
        SimulationOptions(
            stop_time=0.1,
            step_size=0.1,
            outputs=("brakeTorque", "appliedMagnitude"),
        ),
        backend=modelica_backend,
    )
    direction = 20 / math.sqrt(20**2 + 0.1**2)
    assert result["brakeTorque"][-1] == pytest.approx(-2100 * direction, abs=1e-8)
