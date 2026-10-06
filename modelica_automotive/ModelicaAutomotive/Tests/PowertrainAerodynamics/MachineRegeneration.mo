within ModelicaAutomotive.Tests.PowertrainAerodynamics;
model MachineRegeneration "First-order regenerative command"
  ModelicaAutomotive.Powertrain.FirstOrderMachine machine(
    parameters(
      maximumDriveTorque=300,
      maximumRegenerativeTorque=120,
      baseSpeed=300,
      maximumSpeed=600,
      responseTime=0.5,
      driveEfficiency=0.9,
      regenerativeEfficiency=0.8));
  output Real commandState;
  output Real torque;
  output Real mechanicalPower;
  output Real sourcePower;
equation
  machine.command = -1;
  machine.angularVelocity = 200;
  commandState = machine.commandState;
  torque = machine.torque;
  mechanicalPower = machine.mechanicalPower;
  sourcePower = machine.sourcePower;
end MachineRegeneration;
