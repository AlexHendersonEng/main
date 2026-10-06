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


def _array6(result, name: str) -> np.ndarray:
    return np.array([result[f"{name}[{index}]"][-1] for index in range(1, 7)])


def _bretschneider(frequency: float, height: float, period: float) -> float:
    peak_frequency = 1 / period
    return (
        0.3125
        * height**2
        * peak_frequency**4
        / frequency**5
        * math.exp(-1.25 * (peak_frequency / frequency) ** 4)
    )


def _jonswap(frequency: float, height: float, period: float, gamma: float) -> float:
    peak_frequency = 1 / period
    sigma = 0.07 if frequency <= peak_frequency else 0.09
    peak_shape = math.exp(-0.5 * ((frequency - peak_frequency) / (sigma * peak_frequency)) ** 2)
    alpha = 5.061 * height**2 / period**4 * (1 - 0.287 * math.log(gamma))
    return (
        alpha
        * G0**2
        / ((2 * math.pi) ** 4 * frequency**5)
        * math.exp(-1.25 * (peak_frequency / frequency) ** 4)
        * gamma**peak_shape
    )


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


def test_profiles_wind_and_bathymetry(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Tests.Environment.ProfileValidation",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.5, step_size=0.5),
        backend=modelica_backend,
    )

    temperature = 284
    salinity = 0.0355
    density_nodes = np.array(
        [
            1025 * (1 - 2e-4 * (290 - 288) + 0.8 * (0.034 - 0.035)),
            1025 * (1 - 2e-4 * (285 - 288) + 0.8 * (0.035 - 0.035)),
            1025 * (1 - 2e-4 * (283 - 288) + 0.8 * (0.036 - 0.035)),
        ]
    )
    density = 0.5 * (density_nodes[1] + density_nodes[2])
    integrated_density = (
        0.5 * (density_nodes[0] + density_nodes[1]) * 50 + 0.5 * (density_nodes[1] + density) * 25
    )
    expected_current = np.array([0.35, 0.25, 0.05])
    gust = math.sin(2 * math.pi * 0.25 * 0.5 + 0.2)

    assert result["temperature"][-1] == pytest.approx(temperature)
    assert result["salinity"][-1] == pytest.approx(salinity)
    assert result["density"][-1] == pytest.approx(density)
    assert result["pressure"][-1] == pytest.approx(P0 + G0 * integrated_density)
    np.testing.assert_allclose(_array(result, "waterCurrent"), expected_current, atol=1e-12)
    np.testing.assert_allclose(_array(result, "profileCurrent"), expected_current, atol=1e-12)
    np.testing.assert_allclose(
        _array(result, "windVelocity"),
        np.array([8, -1, 0]) + np.array([2, 1, 0.5]) * gust,
        atol=1e-10,
    )
    np.testing.assert_allclose(
        _array(result, "directionalWind"),
        [10 * math.cos(math.pi / 6), 5, -0.2],
        atol=1e-10,
    )
    np.testing.assert_allclose(_array(result, "steadyWindVelocity"), [4, 3, -0.2], atol=1e-12)
    assert result["flatAltitude"][-1] == pytest.approx(50)
    assert result["slopeDepth"][-1] == pytest.approx(118)
    assert result["slopeAltitude"][-1] == pytest.approx(48)


def test_wave_spectra_moments_and_kinematics(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Tests.Environment.WaveValidation",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.5, step_size=0.5),
        backend=modelica_backend,
    )

    frequency = 0.12
    wind_speed = 15
    angular_scale = G0 / (2 * math.pi * wind_speed * frequency)
    pm = 0.0081 * G0**2 / ((2 * math.pi) ** 4 * frequency**5) * math.exp(-0.74 * angular_scale**4)
    assert result["piersonMoskowitz"][-1] == pytest.approx(pm)
    assert result["bretschneider"][-1] == pytest.approx(_bretschneider(frequency, 3, 9))
    assert result["calmBretschneider"][-1] == pytest.approx(0)
    assert result["jonswap"][-1] == pytest.approx(_jonswap(frequency, 3, 9, 3.3))

    regular_omega = 2 * math.pi / 8
    regular_wave_number = regular_omega**2 / G0
    regular_phase = (
        regular_wave_number * (math.cos(0.4) * 10 + math.sin(0.4) * -5) - regular_omega * 0.5 + 0.3
    )
    regular_attenuation = math.exp(-regular_wave_number * 4)
    assert result["regularElevation"][-1] == pytest.approx(1.2 * math.cos(regular_phase))
    np.testing.assert_allclose(
        _array(result, "regularVelocity"),
        [
            1.2 * regular_omega * regular_attenuation * math.cos(regular_phase) * math.cos(0.4),
            1.2 * regular_omega * regular_attenuation * math.cos(regular_phase) * math.sin(0.4),
            -1.2 * regular_omega * regular_attenuation * math.sin(regular_phase),
        ],
        rtol=1e-9,
        atol=1e-10,
    )

    frequencies = np.array([0.08, 0.12, 0.16, 0.20])
    directions = np.array([0, 0.2, -0.1, 0.3])
    phases = np.array([0.1, 1.2, 2.1, 3.0])
    spectrum = np.array([_bretschneider(value, 3, 9) for value in frequencies])
    amplitudes = np.sqrt(2 * spectrum * 0.04)
    omega = 2 * math.pi * frequencies
    wave_number = omega**2 / G0
    position = np.array([10, -5, 4])
    wave_phase = (
        wave_number * (np.cos(directions) * position[0] + np.sin(directions) * position[1])
        - omega * 0.5
        + phases
    )
    attenuation = np.exp(-wave_number * position[2])
    expected_elevation = np.sum(amplitudes * np.cos(wave_phase))
    expected_velocity = np.array(
        [
            np.sum(amplitudes * omega * attenuation * np.cos(wave_phase) * np.cos(directions)),
            np.sum(amplitudes * omega * attenuation * np.cos(wave_phase) * np.sin(directions)),
            np.sum(-amplitudes * omega * attenuation * np.sin(wave_phase)),
        ]
    )
    moment_zero = np.sum(spectrum) * 0.04

    np.testing.assert_allclose(
        np.array([result[f"spectrum[{index}]"][-1] for index in range(1, 5)]),
        spectrum,
        rtol=1e-10,
    )
    assert result["momentZero"][-1] == pytest.approx(moment_zero)
    assert result["realizedHeight"][-1] == pytest.approx(4 * math.sqrt(moment_zero))
    assert result["irregularElevation"][-1] == pytest.approx(expected_elevation)
    np.testing.assert_allclose(
        _array(result, "irregularVelocity"), expected_velocity, rtol=1e-9, atol=1e-10
    )

    seeded_frequencies = np.array([0.08, 0.12, 0.16])
    seeded_amplitudes = np.sqrt(
        2 * np.array([_bretschneider(value, 3, 9) for value in seeded_frequencies]) * 0.04
    )
    seeded_phases = np.array(
        [
            2 * math.pi * (0.6180339887498949 * index + 0.4142135623730950 * 7)
            for index in range(1, 4)
        ]
    )
    expected_seeded_elevation = np.sum(
        seeded_amplitudes * np.cos(-2 * math.pi * seeded_frequencies * 0.5 + seeded_phases)
    )
    assert result["seededElevation"][-1] == pytest.approx(expected_seeded_elevation)


def test_environmental_relative_loads(modelica_backend: str):
    model = Model(
        "ModelicaMaritime.Tests.Environment.EnvironmentalLoadValidation",
        files=library_model_files(modelica_backend),
        libraries=("Modelica",),
    )
    result = model.simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )

    velocity = np.array([5, -2, 1])
    force = -0.5 * 1.225 * np.array([0.8, 1.1, 1.3]) * np.array([12, 20, 8])
    force *= np.abs(velocity) * velocity
    moment = np.cross(np.array([2, 0, -3]), force)
    expected_drag = np.concatenate((force, moment))
    expected_wave = np.array(
        [
            100 * 1.5 + 10 * 0.4,
            200 * 1.5 + 20 * -0.2,
            30 * 0.1,
            0,
            0,
            50 * 1.5 + 5 * 0.4 - 5 * -0.2,
        ]
    )

    np.testing.assert_allclose(_array6(result, "dragLoad"), expected_drag, atol=1e-10)
    np.testing.assert_allclose(_array6(result, "waveLoad"), expected_wave, atol=1e-10)
