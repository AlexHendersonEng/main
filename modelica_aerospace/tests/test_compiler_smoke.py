from __future__ import annotations

import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_FILE, PACKAGE_SOURCES

pytestmark = pytest.mark.integration


def test_package_loads_and_simulates(modelica_backend: str):
    sources = PACKAGE_SOURCES if modelica_backend == "rumoca" else (PACKAGE_FILE,)
    model = Model(
        "ModelicaAerospace.Examples.PackageSmoke",
        files=sources,
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1, outputs=("y",)),
        parameters={"scale": 3.5},
        backend=modelica_backend,
    )

    assert result["y"] == pytest.approx([3.5, 3.5])
    assert result.metadata["backend"] == modelica_backend
