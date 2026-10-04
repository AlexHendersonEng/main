from __future__ import annotations

import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_FILE, PACKAGE_ROOT, PROJECT_ROOT

pytestmark = pytest.mark.integration

COMMON_VALIDATION = PROJECT_ROOT / "tests" / "modelica" / "CommonValidation.mo"


def _common_model(backend: str) -> Model:
    files = (
        (COMMON_VALIDATION, PACKAGE_ROOT.parent)
        if backend == "rumoca"
        else (PACKAGE_FILE, COMMON_VALIDATION)
    )
    return Model("CommonValidation", files=files, libraries=("Modelica",))


def test_common_definitions_compile_and_simulate(modelica_backend: str):
    result = _common_model(modelica_backend).simulate(
        SimulationOptions(
            stop_time=0.1,
            step_size=0.1,
        ),
        backend=modelica_backend,
    )

    assert result["y"] == pytest.approx([6, 6])
    assert result["vectorY[1]"] == pytest.approx([2, 2])
    assert result["vectorY[3]"] == pytest.approx([6, 6])
    assert result["environmentDensity"] == pytest.approx([1.225, 1.225])
    assert result["sensorMeasurement[2]"] == pytest.approx([5, 5])
    assert result["vehicleVelocity[3]"] == pytest.approx([9, 9])
    assert result["semiMajorAxis"] == pytest.approx([6378137, 6378137])
    assert result["defaultQuaternionScalar"] == pytest.approx([1, 1])


def test_public_type_metadata_and_validation_guards_are_declared():
    angle = (PACKAGE_ROOT / "Types" / "Angle.mo").read_text(encoding="utf-8")
    mass = (PACKAGE_ROOT / "Types" / "Mass.mo").read_text(encoding="utf-8")
    positive = (PACKAGE_ROOT / "Utilities" / "assertPositive.mo").read_text(encoding="utf-8")
    bounded = (PACKAGE_ROOT / "Utilities" / "assertInRange.mo").read_text(encoding="utf-8")

    assert 'quantity="Angle"' in angle and 'unit="rad"' in angle
    assert 'quantity="Mass"' in mass and 'unit="kg"' in mass and "min=0" in mass
    assert "assert(value > 0," in positive
    assert "value >= minimum and value <= maximum" in bounded


def test_readme_documents_stable_frame_and_signal_conventions():
    readme = (PROJECT_ROOT / "README.md").read_text(encoding="utf-8")

    for convention in (
        "x forward, y starboard, z down",
        "north, east, down (NED)",
        "scalar-first `{w, x, y, z}`",
        "`{p, q, r}`",
        "`{CX, CY, CZ}`",
        "SI units",
    ):
        assert convention in readme
