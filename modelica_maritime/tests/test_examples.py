from __future__ import annotations

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration


def test_surface_maneuvering_example_is_bounded(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Examples.SurfaceManeuvering",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=60, step_size=0.1),
        backend=modelica_backend,
    )

    north = np.asarray(result["poseNED[1]"])
    east = np.asarray(result["poseNED[2]"])
    heading = np.asarray(result["poseNED[3]"])
    surge = np.asarray(result["velocityBody[1]"])
    sway = np.asarray(result["velocityBody[2]"])
    yaw_rate = np.asarray(result["velocityBody[3]"])
    dissipation = np.asarray(result["dissipationPower"])

    assert north[-1] > 50
    assert abs(east[-1]) > 3
    assert abs(heading[-1]) > 0.05
    assert np.max(np.abs(surge)) < 10
    assert np.max(np.abs(sway)) < 5
    assert np.max(np.abs(yaw_rate)) < 0.4
    assert np.all(dissipation >= -1e-8)
