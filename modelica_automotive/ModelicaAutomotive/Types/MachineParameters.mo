within ModelicaAutomotive.Types;
record MachineParameters "First-order engine or motor parameters"
  ModelicaAutomotive.Types.Torque maximumDriveTorque = 300;
  ModelicaAutomotive.Types.Torque maximumRegenerativeTorque = 150;
  ModelicaAutomotive.Types.AngularVelocity baseSpeed = 300;
  ModelicaAutomotive.Types.AngularVelocity maximumSpeed = 600;
  Real responseTime(unit="s") = 0.15;
  Real driveEfficiency(min=ModelicaAutomotive.Constants.small, max=1) = 0.92;
  Real regenerativeEfficiency(min=0, max=1) = 0.75;
end MachineParameters;
