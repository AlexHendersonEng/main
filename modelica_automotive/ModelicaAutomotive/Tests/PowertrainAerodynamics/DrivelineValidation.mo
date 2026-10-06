within ModelicaAutomotive.Tests.PowertrainAerodynamics;
model DrivelineValidation "Validate ratios, differential, torque split, shaft, and clutch"
  ModelicaAutomotive.Powertrain.FixedRatio fixed(
    ratio=4,
    efficiency=0.9);
  ModelicaAutomotive.Powertrain.SelectableTransmission selectable(
    ratios={10, 6, 4},
    efficiency=0.95);
  ModelicaAutomotive.Powertrain.OpenDifferential differential(
    finalDriveRatio=3,
    efficiency=0.9);
  ModelicaAutomotive.Powertrain.TorqueDistributor distributor(frontFraction=0.4);
  ModelicaAutomotive.Powertrain.DriveshaftCompliance shaft(
    stiffness=1000,
    damping=20);
  ModelicaAutomotive.Powertrain.FrictionClutch clutch(
    maximumTorque=400,
    slipRegularization=0.5);
  output Real fixedInputSpeed;
  output Real fixedOutputTorque;
  output Real fixedPowerLoss;
  output Real selectedRatio;
  output Real selectedOutputTorque;
  output Real differentialInputSpeed;
  output Real differentialTorque[2];
  output Real differentialPowerLoss;
  output Real distributedTorque[2];
  output Real shaftTorque;
  output Real shaftEnergy;
  output Real shaftDissipation;
  output Real clutchTorque[2];
  output Real clutchDissipation;
equation
  fixed.inputTorque = 100;
  fixed.outputAngularVelocity = 50;
  selectable.gearCommand = 2;
  selectable.inputTorque = 100;
  selectable.outputAngularVelocity = 40;
  differential.inputTorque = 120;
  differential.leftAngularVelocity = 30;
  differential.rightAngularVelocity = 34;
  distributor.inputTorque = 500;
  shaft.inputAngle = 0.1;
  shaft.outputAngle = 0.04;
  shaft.inputAngularVelocity = 20;
  shaft.outputAngularVelocity = 18;
  clutch.command = 0.5;
  clutch.inputAngularVelocity = 30;
  clutch.outputAngularVelocity = 20;
  fixedInputSpeed = fixed.inputAngularVelocity;
  fixedOutputTorque = fixed.outputTorque;
  fixedPowerLoss = fixed.inputPower - fixed.outputPower;
  selectedRatio = selectable.selectedRatio;
  selectedOutputTorque = selectable.outputTorque;
  differentialInputSpeed = differential.inputAngularVelocity;
  differentialTorque = {differential.leftTorque, differential.rightTorque};
  differentialPowerLoss = differential.inputPower - differential.outputPower;
  distributedTorque = {distributor.frontTorque, distributor.rearTorque};
  shaftTorque = shaft.transmittedTorque;
  shaftEnergy = shaft.storedEnergy;
  shaftDissipation = shaft.dissipatedPower;
  clutchTorque = {clutch.inputTorque, clutch.outputTorque};
  clutchDissipation = clutch.dissipatedPower;
end DrivelineValidation;
