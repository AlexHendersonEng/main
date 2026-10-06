from __future__ import annotations

import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration


def test_common_definitions_compile_and_simulate(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Tests.Common.CommonValidation",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    assert result["environmentDensity"] == pytest.approx([1025, 1025])
    assert result["sensorMeasurement"] == pytest.approx([4, 4])
    assert result["planarVelocity[1]"] == pytest.approx([7, 7])
    assert result["rigidVelocity[6]"] == pytest.approx([6, 6])
    assert result["standardGravity"] == pytest.approx([9.80665, 9.80665])
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
    readme = (PACKAGE_ROOT.parent / "README.md").read_text(encoding="utf-8")
    for convention in (
        "x forward, y starboard, z down",
        "north, east, down (NED)",
        "`nu = {u, v, w, p, q, r}`",
        "`tau = {X, Y, Z, K, M, N}`",
        "Water-relative velocity",
        "SI units",
    ):
        assert convention in readme
