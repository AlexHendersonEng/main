from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration

G0 = 9.80665
P0 = 101325.0


def _array(result, name: str) -> np.ndarray:
    return np.array([result[f"{name}[{index}]"][-1] for index in range(1, 4)])


def test_water_properties_pressure_and_currents(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Tests.Environment.EnvironmentValidation",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    expected_density = 1025 * (1 - 2e-4 * (283.15 - 288.15) + 0.8 * (0.036 - 0.035))
    assert result["functionDensity"][-1] == pytest.approx(expected_density)
    assert result["functionPressure"][-1] == pytest.approx(P0 + 1025 * G0 * 50)
    np.testing.assert_allclose(
        _array(result, "directionalCurrent"),
        [2 * math.cos(math.pi / 6), 1, 0.1],
        atol=1e-12,
    )
    expected_profile = np.array([1, 0.5, 0]) + np.array([-0.01, 0.02, 0.001]) * 20
    np.testing.assert_allclose(_array(result, "profileCurrent"), expected_profile, atol=1e-12)

    assert result["constantTemperature"][-1] == pytest.approx(290)
    assert result["constantDensity"][-1] == pytest.approx(1027)
    assert result["constantPressure"][-1] == pytest.approx(P0 + 1027 * G0 * 20)
    np.testing.assert_allclose(_array(result, "constantCurrent"), [1, -0.5, 0.1], atol=1e-12)

    linear_temperature = 290 - 0.02 * 20
    salinity_change = 2e-5 * 20
    linear_density = 1024 * (1 - 2e-4 * (linear_temperature - 290) + 0.8 * salinity_change)
    assert result["linearTemperature"][-1] == pytest.approx(linear_temperature)
    assert result["linearDensity"][-1] == pytest.approx(linear_density)
    relative_density_gradient = -2e-4 * -0.02 + 0.8 * 2e-5
    integrated_density = 1024 * (20 + 0.5 * relative_density_gradient * 20**2)
    assert result["linearPressure"][-1] == pytest.approx(P0 + G0 * integrated_density)
    np.testing.assert_allclose(_array(result, "linearCurrent"), expected_profile, atol=1e-12)
    np.testing.assert_allclose(_array(result, "steadyBlockCurrent"), [2, -1, 0.2], atol=1e-12)
    np.testing.assert_allclose(_array(result, "linearBlockCurrent"), expected_profile, atol=1e-12)
