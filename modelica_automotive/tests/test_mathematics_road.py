from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration


def test_mathematics_and_road_queries_match_independent_references(
    modelica_backend: str,
):
    model = Model(
        "ModelicaAutomotive.Tests.MathematicsRoad.MathematicsRoadValidation",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    expected_heights = [
        10 + 2 * math.tan(0.1) + 0.8 * math.tan(0.05) + 0.02 * 0.8**2,
        10 + 2 * math.tan(0.1) - 0.8 * math.tan(0.05) + 0.02 * 0.8**2,
        10 - math.tan(0.1) + 0.8 * math.tan(0.05) + 0.02 * 0.8**2,
        10 - math.tan(0.1) - 0.8 * math.tan(0.05) + 0.02 * 0.8**2,
    ]
    actual_heights = [result[f"roadHeights[{index}]"][-1] for index in range(1, 5)]
    np.testing.assert_allclose(actual_heights, expected_heights, atol=1e-10)
    assert result["wrapped"][-1] == pytest.approx(4 - 2 * math.pi, abs=1e-12)
    assert result["zeroSign"][-1] == pytest.approx(0, abs=1e-12)
    assert result["positiveSign"][-1] == pytest.approx(1 / math.sqrt(1.01), abs=1e-12)

    tangent = np.array(
        [math.cos(0.3) * math.cos(0.1), math.sin(0.3) * math.cos(0.1), math.sin(0.1)]
    )
    transformed = np.array([result[f"transformedVector[{index}]"][-1] for index in range(1, 4)])
    normal = np.array([result[f"roadNormal[{index}]"][-1] for index in range(1, 4)])
    np.testing.assert_allclose(transformed, tangent, atol=1e-12)
    assert np.linalg.norm(normal) == pytest.approx(1, abs=1e-12)
    assert np.dot(normal, tangent) == pytest.approx(0, abs=1e-12)
    np.testing.assert_allclose(
        [result[f"aggregateForce[{index}]"][-1] for index in range(1, 4)],
        [40, 0, 0],
        atol=1e-12,
    )
    np.testing.assert_allclose(
        [result[f"aggregateMoment[{index}]"][-1] for index in range(1, 4)],
        [0, 0, 0],
        atol=1e-12,
    )


def test_road_models_declare_singularity_and_friction_guards():
    sources = [
        PACKAGE_ROOT / "Road" / "roadHeight.mo",
        PACKAGE_ROOT / "Road" / "ConstantRoad.mo",
        PACKAGE_ROOT / "Road" / "FourCornerRoad.mo",
    ]
    text = "\n".join(path.read_text(encoding="utf-8") for path in sources)
    for guard in (
        "assert(abs(road.grade) < 1.5707963267948966,",
        "assert(abs(road.bank) < 1.5707963267948966,",
        "assert(road.frictionCoefficient >= 0,",
    ):
        assert guard in text
