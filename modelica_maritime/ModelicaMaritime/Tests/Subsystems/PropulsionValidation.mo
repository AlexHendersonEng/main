within ModelicaMaritime.Tests.Subsystems;
model PropulsionValidation "Validate propeller, thrusters, shaft lag, and energy state"
  ModelicaMaritime.Propulsion.OpenWaterPropeller propeller(
    density=1025,
    diameter=2,
    thrustCoefficient={0.3, -0.1, 0.02},
    torqueCoefficient={0.04, -0.01, 0.005},
    wakeFraction=0.2,
    thrustDeduction=0.1);
  ModelicaMaritime.Propulsion.OpenWaterPropeller reversePropeller(
    density=1025,
    diameter=2,
    thrustCoefficient={0.3, -0.1, 0.02},
    torqueCoefficient={0.04, -0.01, 0.005},
    wakeFraction=0.2,
    thrustDeduction=0.1);
  ModelicaMaritime.Propulsion.FixedThruster fixed(
    maximumForwardThrust=1000,
    maximumReverseThrust=600,
    directionBody={1, 0, 0},
    applicationPointBody={0, 2, 0});
  ModelicaMaritime.Propulsion.AzimuthThruster azimuth(
    maximumForwardThrust=1000,
    maximumReverseThrust=500,
    applicationPointBody={-1, 0, 0});
  ModelicaMaritime.Propulsion.FixedThruster failed(
    maximumForwardThrust=1000);
  ModelicaMaritime.Propulsion.FirstOrderShaft shaft(
    timeConstant=0.5,
    maximumForwardRate=20,
    maximumReverseRate=10);
  ModelicaMaritime.Propulsion.EnergyStore store(
    capacity=1000,
    initialEnergy=500,
    maximumDischargePower=200,
    maximumChargePower=100);
  ModelicaMaritime.Propulsion.EnergyStore emptyStore(
    capacity=1000,
    initialEnergy=50,
    maximumDischargePower=200,
    maximumChargePower=100);
  ModelicaMaritime.Propulsion.EnergyStore fullStore(
    capacity=1000,
    initialEnergy=950,
    maximumDischargePower=200,
    maximumChargePower=100);
  output Real advanceRatio;
  output Real propellerThrust;
  output Real propellerTorque;
  output Real shaftPower;
  output Real reverseThrust;
  output Real reverseTorque;
  output Real reversePower;
  output Real fixedLoad[6];
  output Real azimuthLoad[6];
  output Real failedLoad[6];
  output Real shaftRate;
  output Real energy;
  output Real stateOfCharge;
  output Real emptyEnergy;
  output Real emptyPower;
  output Real fullEnergy;
  output Real fullPower;
equation
  propeller.shaftRate = 10;
  propeller.advanceVelocity = 4;
  reversePropeller.shaftRate = -10;
  reversePropeller.advanceVelocity = -4;
  fixed.command = 0.5;
  fixed.enabled = true;
  fixed.failed = false;
  azimuth.command = -0.4;
  azimuth.azimuth = 1.5707963267948966;
  azimuth.enabled = true;
  azimuth.failed = false;
  failed.command = 1;
  failed.enabled = true;
  failed.failed = true;
  shaft.command = if time < 1 then 0 else 0.8;
  shaft.enabled = true;
  shaft.failed = false;
  store.requestedPower = if time < 2 then 100 else -50;
  store.enabled = true;
  emptyStore.requestedPower = 100;
  emptyStore.enabled = true;
  fullStore.requestedPower = -100;
  fullStore.enabled = true;
  advanceRatio = propeller.advanceRatio;
  propellerThrust = propeller.thrust;
  propellerTorque = propeller.torque;
  shaftPower = propeller.shaftPower;
  reverseThrust = reversePropeller.thrust;
  reverseTorque = reversePropeller.torque;
  reversePower = reversePropeller.shaftPower;
  fixedLoad = fixed.generalizedLoadBody;
  azimuthLoad = azimuth.generalizedLoadBody;
  failedLoad = failed.generalizedLoadBody;
  shaftRate = shaft.shaftRate;
  energy = store.energy;
  stateOfCharge = store.stateOfCharge;
  emptyEnergy = emptyStore.energy;
  emptyPower = emptyStore.deliveredPower;
  fullEnergy = fullStore.energy;
  fullPower = fullStore.deliveredPower;
end PropulsionValidation;
