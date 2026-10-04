from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import library_model_files

pytestmark = pytest.mark.integration

G0 = 9.80665
R = 287.05287
GAMMA = 1.4
GEOPOTENTIAL_RADIUS = 6_356_766.0
WGS84_A = 6_378_137.0
WGS84_E2 = 6.6943799901413165e-3
WGS84_MU = 3.986004418e14
EARTH_RATE = 7.292115e-5

LAYERS = (
    (0.0, 288.15, 101325.0, -0.0065),
    (11000.0, 216.65, 22632.0400950078, 0.0),
    (20000.0, 216.65, 5474.87742428105, 0.001),
    (32000.0, 228.65, 868.015776620216, 0.0028),
    (47000.0, 270.65, 110.90577336731, 0.0),
    (51000.0, 270.65, 66.9385281211797, -0.0028),
    (71000.0, 214.65, 3.95639216039661, -0.002),
)


def _environment_model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAerospace.Tests.Environment.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, final: bool = True) -> np.ndarray:
    values = [np.asarray(result[f"{name}[{index}]"]) for index in range(1, 4)]
    return np.array([value[-1] for value in values]) if final else np.vstack(values).T


def _geometric_from_geopotential(altitude: float) -> float:
    return GEOPOTENTIAL_RADIUS * altitude / (GEOPOTENTIAL_RADIUS - altitude)


def _atmosphere(geometric_altitude: float) -> tuple[float, float, float, float, float]:
    altitude = GEOPOTENTIAL_RADIUS * geometric_altitude / (GEOPOTENTIAL_RADIUS + geometric_altitude)
    base_altitude, base_temperature, base_pressure, lapse = max(
        layer for layer in LAYERS if layer[0] <= altitude
    )
    temperature = base_temperature + lapse * (altitude - base_altitude)
    if lapse == 0:
        pressure = base_pressure * math.exp(
            -G0 * (altitude - base_altitude) / (R * base_temperature)
        )
    else:
        pressure = base_pressure * (base_temperature / temperature) ** (G0 / (R * lapse))
    density = pressure / (R * temperature)
    speed_of_sound = math.sqrt(GAMMA * R * temperature)
    viscosity = 1.716e-5 * (temperature / 273.15) ** 1.5 * (273.15 + 110.4) / (temperature + 110.4)
    return temperature, pressure, density, speed_of_sound, viscosity


@pytest.mark.parametrize(
    ("class_name", "geopotential_altitude", "latitude"),
    [
        ("SeaLevel", 0.0, 0.0),
        ("Tropopause", 11000.0, 0.7),
        ("Stratopause", 47000.0, 0.7),
        ("Mesopause", 84852.0, math.pi / 2),
    ],
)
def test_standard_atmosphere_and_gravity_references(
    modelica_backend: str,
    class_name: str,
    geopotential_altitude: float,
    latitude: float,
):
    geometric_altitude = _geometric_from_geopotential(geopotential_altitude)
    result = _environment_model(modelica_backend, class_name).simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    expected = _atmosphere(geometric_altitude)
    probe = 1e-7

    for name, value in zip(
        ("temperature", "pressure", "density", "speedOfSound", "dynamicViscosity"),
        expected,
        strict=True,
    ):
        assert result[name][-1] == pytest.approx(value + probe, rel=2e-10, abs=1e-10)

    assert result["mach"][-1] == pytest.approx(250 / expected[3] + probe, rel=1e-10)
    assert result["dynamicPressure"][-1] == pytest.approx(0.5 * expected[2] * 250**2 + probe)
    assert result["inverseSquareGravity"][-1] == pytest.approx(
        WGS84_MU / (WGS84_A + geometric_altitude) ** 2 + probe
    )

    sin_latitude = math.sin(latitude)
    surface_gravity = (
        9.7803253359
        * (1 + 0.00193185265241 * sin_latitude**2)
        / math.sqrt(1 - WGS84_E2 * sin_latitude**2)
    )
    expected_normal = surface_gravity * (WGS84_A / (WGS84_A + geometric_altitude)) ** 2
    assert result["normalGravity"][-1] == pytest.approx(expected_normal + probe, rel=1e-10)
    assert _array(result, "blockGravity") == pytest.approx(
        [probe, 2 * probe, expected_normal + 3 * probe]
    )
    assert result["blockTemperature"][-1] == pytest.approx(expected[0] + probe)


def test_atmosphere_layer_boundaries_are_continuous(modelica_backend: str):
    for class_name in ("Tropopause", "Stratopause"):
        result = _environment_model(modelica_backend, class_name).simulate(
            SimulationOptions(stop_time=0.1, step_size=0.1),
            backend=modelica_backend,
        )
        assert np.all(np.isfinite(result["pressure"]))
        assert np.all(np.asarray(result["pressure"]) > 0)
        assert np.all(np.asarray(result["density"]) > 0)


def test_gravity_rotation_and_zero_wind_references(modelica_backend: str):
    environment = _environment_model(modelica_backend, "SeaLevel").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    probe = 1e-7
    assert _array(environment, "constantGravity") == pytest.approx(
        [probe, 2 * probe, G0 + 3 * probe]
    )
    assert _array(environment, "transportVelocity") == pytest.approx(
        [probe, EARTH_RATE * WGS84_A + 2 * probe, 3 * probe]
    )
    assert _array(environment, "centrifugalAcceleration") == pytest.approx(
        [EARTH_RATE**2 * WGS84_A + probe, 2 * probe, 3 * probe]
    )

    wind = _environment_model(modelica_backend, "WindValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    assert _array(wind, "steadyWind") == pytest.approx([12, -3, 0])
    assert _array(wind, "shearWind") == pytest.approx([12.5, -1.25, 0])
    assert _array(wind, "gustWind") == pytest.approx([0, 0, 0])


def test_one_minus_cosine_gust_shape_and_seeded_forcing(modelica_backend: str):
    result = _environment_model(modelica_backend, "WindValidation").simulate(
        SimulationOptions(stop_time=8, step_size=0.1),
        backend=modelica_backend,
    )
    time = np.asarray(result["time"])
    gust = _array(result, "gustWind", final=False)
    forcing = _array(result, "forcing", final=False)

    peak = int(np.argmin(np.abs(time - 4)))
    assert gust[peak] == pytest.approx([6, 0, -2], abs=1e-8)
    assert gust[0] == pytest.approx([0, 0, 0])
    assert gust[-1] == pytest.approx([0, 0, 0])

    expected = (
        sum(
            math.sin((0.37 * index + 0.11 + 0.013 * 17) * time[-1] + 0.754877666 * 17 * (index + 1))
            for index in range(1, 9)
        )
        / 2
    )
    assert forcing[-1, 0] == pytest.approx(expected, abs=1e-9)


def test_dryden_traces_are_repeatable_seeded_and_low_pass(modelica_backend: str):
    options = SimulationOptions(stop_time=120, step_size=0.1)
    first = _environment_model(modelica_backend, "WindValidation").simulate(
        options,
        backend=modelica_backend,
    )
    second = _environment_model(modelica_backend, "WindValidation").simulate(
        options,
        backend=modelica_backend,
    )
    turbulence = _array(first, "turbulence", final=False)
    repeated = _array(second, "turbulence", final=False)
    other = _array(first, "otherTurbulence", final=False)
    forcing = _array(first, "forcing", final=False)

    np.testing.assert_allclose(turbulence, repeated, atol=1e-10, rtol=0)
    assert np.max(np.abs(turbulence - other)) > 0.1

    settled = turbulence[len(turbulence) // 4 :]
    sigma = _array(first, "drydenSigma")
    rms = np.sqrt(np.mean(settled**2, axis=0))
    assert np.all(rms > 0.05 * sigma)
    assert np.all(rms < 2.0 * sigma)

    frequencies = np.fft.rfftfreq(len(settled), 0.1)
    turbulent_power = np.abs(np.fft.rfft(settled[:, 0] - settled[:, 0].mean())) ** 2
    forcing_settled = forcing[len(forcing) // 4 :, 0]
    forcing_power = np.abs(np.fft.rfft(forcing_settled - forcing_settled.mean())) ** 2
    high_frequency = frequencies >= 0.5
    assert turbulent_power[high_frequency].sum() < forcing_power[high_frequency].sum()
