within ModelicaAutomotive.Tests.PowertrainAerodynamics;
model EnergyStorageValidation "Constant power discharge and charge accounting"
  ModelicaAutomotive.Powertrain.EnergyStorage discharge(
    capacity=1000000,
    initialStateOfCharge=0.8);
  ModelicaAutomotive.Powertrain.EnergyStorage charge(
    capacity=1000000,
    initialStateOfCharge=0.4);
  output Real dischargeEnergy;
  output Real chargeEnergy;
  output Real dischargeStateOfCharge;
  output Real chargeStateOfCharge;
equation
  discharge.sourcePower = 10000;
  charge.sourcePower = -5000;
  dischargeEnergy = discharge.energy;
  chargeEnergy = charge.energy;
  dischargeStateOfCharge = discharge.stateOfCharge;
  chargeStateOfCharge = charge.stateOfCharge;
end EnergyStorageValidation;
