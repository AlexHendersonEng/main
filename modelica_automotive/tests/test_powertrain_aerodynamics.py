from __future__ import annotations

import math

import numpy as np
import pytest
from polaris import Model, SimulationOptions

from conftest import PACKAGE_ROOT, library_model_files

pytestmark = pytest.mark.integration


def _model(backend: str, class_name: str) -> Model:
    return Model(
        f"ModelicaAutomotive.Tests.PowertrainAerodynamics.{class_name}",
        files=library_model_files(backend),
        libraries=("Modelica",),
    )


def _array(result, name: str, width: int) -> np.ndarray:
    return np.array([result[f"{name}[{index}]"][-1] for index in range(1, width + 1)])


def test_aerodynamic_loads_match_independent_coefficient_scaling(
    modelica_backend: str,
):
    result = _model(modelica_backend, "AerodynamicLoads").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    velocity = np.array([30.0, 2.0, 0.0])
    dynamic_pressure = 0.5 * 1.2 * float(np.dot(velocity, velocity))
    reference_speed = math.sqrt(30**2 + 0.1**2)
    sideslip = math.atan2(2, reference_speed)
    sign = 30 / reference_speed
    force = dynamic_pressure * 2 * np.array([-0.3 * sign, -0.8 * sideslip, -0.1])
    coefficient_moment = dynamic_pressure * 2 * 2.5 * np.array([0.01, -0.02, 0.03])
    expected_moment = coefficient_moment + np.cross(np.array([1, 0, 0.2]), force)

    assert result["dynamicPressure"][-1] == pytest.approx(dynamic_pressure, abs=1e-10)
    actual_sideslip = (
        result["sideslip"][-1]
        if "sideslip" in result.variables
        else result["sideslipIntegral"][-1] / 0.1
    )
    assert actual_sideslip == pytest.approx(sideslip, abs=2e-10)
    np.testing.assert_allclose(_array(result, "forceBody", 3), force, atol=1e-8)
    np.testing.assert_allclose(_array(result, "momentBody", 3), expected_moment, atol=1e-8)


@pytest.mark.parametrize(
    ("class_name", "command", "maximum_torque", "efficiency"),
    [
        ("MachineDrive", 1, 300, 0.9),
        ("MachineRegeneration", -1, 120, 0.8),
    ],
)
def test_first_order_machine_power_and_efficiency(
    modelica_backend: str,
    class_name: str,
    command: float,
    maximum_torque: float,
    efficiency: float,
):
    result = _model(modelica_backend, class_name).simulate(
        SimulationOptions(stop_time=1, step_size=0.02, tolerance=1e-9),
        backend=modelica_backend,
    )
    command_state = command * (1 - math.exp(-2))
    torque = maximum_torque * command_state
    mechanical_power = torque * 200
    source_power = (
        mechanical_power / efficiency if mechanical_power >= 0 else mechanical_power * efficiency
    )
    assert result["commandState"][-1] == pytest.approx(command_state, abs=2e-6)
    assert result["torque"][-1] == pytest.approx(torque, abs=5e-4)
    assert result["mechanicalPower"][-1] == pytest.approx(mechanical_power, abs=0.1)
    assert result["sourcePower"][-1] == pytest.approx(source_power, abs=0.1)


def test_energy_storage_accounts_for_discharge_and_charge(modelica_backend: str):
    result = _model(modelica_backend, "EnergyStorageValidation").simulate(
        SimulationOptions(stop_time=10, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["dischargeEnergy"][-1] == pytest.approx(700000, abs=1e-5)
    assert result["chargeEnergy"][-1] == pytest.approx(450000, abs=1e-5)
    assert result["dischargeStateOfCharge"][-1] == pytest.approx(0.7, abs=1e-10)
    assert result["chargeStateOfCharge"][-1] == pytest.approx(0.45, abs=1e-10)


def test_driveline_ratios_power_losses_and_dissipation(modelica_backend: str):
    result = _model(modelica_backend, "DrivelineValidation").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["fixedInputSpeed"][-1] == pytest.approx(200, abs=1e-12)
    assert result["fixedOutputTorque"][-1] == pytest.approx(360, abs=1e-12)
    assert result["fixedPowerLoss"][-1] == pytest.approx(2000, abs=1e-9)
    assert result["selectedRatio"][-1] == pytest.approx(6, abs=1e-12)
    assert result["selectedOutputTorque"][-1] == pytest.approx(570, abs=1e-12)
    assert result["differentialInputSpeed"][-1] == pytest.approx(96, abs=1e-12)
    np.testing.assert_allclose(_array(result, "differentialTorque", 2), [162, 162], atol=1e-12)
    assert result["differentialPowerLoss"][-1] == pytest.approx(1152, abs=1e-9)
    np.testing.assert_allclose(_array(result, "distributedTorque", 2), [200, 300], atol=1e-12)
    assert result["shaftTorque"][-1] == pytest.approx(100, abs=1e-12)
    assert result["shaftEnergy"][-1] == pytest.approx(1.8, abs=1e-12)
    assert result["shaftDissipation"][-1] == pytest.approx(80, abs=1e-12)
    clutch_direction = 10 / math.sqrt(10**2 + 0.5**2)
    np.testing.assert_allclose(
        _array(result, "clutchTorque", 2),
        [-200 * clutch_direction, 200 * clutch_direction],
        atol=1e-10,
    )
    assert result["clutchDissipation"][-1] == pytest.approx(
        2000 * clutch_direction,
        abs=1e-9,
    )


def test_top_speed_force_balance_preserves_target_speed(modelica_backend: str):
    result = _model(modelica_backend, "TopSpeedEquilibrium").simulate(
        SimulationOptions(stop_time=5, step_size=0.05, tolerance=1e-9),
        backend=modelica_backend,
    )
    assert result["acceleration"][-1] == pytest.approx(0, abs=2e-10)
    assert result["netForce"][-1] == pytest.approx(0, abs=2e-7)
    assert result["driveForce"][-1] > 500


def test_mapped_components_interpolate_with_msl_tables(modelica_backend: str):
    if modelica_backend == "rumoca":
        pytest.skip("Rumoca 0.10 cannot lower the MSL native table constructor")
    result = _model(modelica_backend, "MappedComponents").simulate(
        SimulationOptions(stop_time=0.1, step_size=0.1),
        backend=modelica_backend,
    )
    assert result["torque"][-1] == pytest.approx(120, abs=1e-10)
    drag = (
        result["dragCoefficient"][-1]
        if "dragCoefficient" in result.variables
        else result["dragIntegral"][-1] / 0.1
    )
    side = (
        result["sideCoefficient"][-1]
        if "sideCoefficient" in result.variables
        else result["sideIntegral"][-1] / 0.1
    )
    lift = (
        result["liftCoefficient"][-1]
        if "liftCoefficient" in result.variables
        else result["liftIntegral"][-1] / 0.1
    )
    assert drag == pytest.approx(0.31, abs=2e-10)
    assert side == pytest.approx(0, abs=2e-10)
    assert lift == pytest.approx(-0.025, abs=2e-10)


def test_powertrain_and_aerodynamics_declare_parameter_guards():
    sources = [
        PACKAGE_ROOT / "Aerodynamics" / "BodyLoads.mo",
        PACKAGE_ROOT / "Powertrain" / "FirstOrderMachine.mo",
        PACKAGE_ROOT / "Powertrain" / "EnergyStorage.mo",
        PACKAGE_ROOT / "Powertrain" / "FixedRatio.mo",
        PACKAGE_ROOT / "Powertrain" / "FrictionClutch.mo",
    ]
    text = "\n".join(path.read_text(encoding="utf-8") for path in sources)
    for guard in (
        "assert(parameters.referenceArea >= 0,",
        "assert(parameters.maximumSpeed > parameters.baseSpeed,",
        "assert(parameters.responseTime > 0,",
        "assert(capacity > 0,",
        "assert(ratio > 0,",
        "assert(slipRegularization > 0,",
    ):
        assert guard in text
