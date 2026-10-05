from __future__ import annotations

import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def test_package_loads_and_simulates(modelica_backend: str):
    model = Model(
        "ModelicaAutomotive.Examples.PackageSmoke",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1, outputs=("y",)),
        parameters={"scale": 3.5},
        backend=modelica_backend,
    )

    assert result["y"] == pytest.approx([3.5, 3.5])
    assert result.metadata["backend"] == modelica_backend
