within ModelicaAutomotive.Tests.WheelsBrakes;
model MappedBrakeEvaluation "Evaluate MSL table-mapped brake torque"
  ModelicaAutomotive.Brakes.MappedBrake brake;
  output Real brakeTorque;
  output Real appliedMagnitude;
equation
  brake.command = 0.75;
  brake.angularVelocity = 20;
  brakeTorque = brake.brakeTorque;
  appliedMagnitude = brake.appliedMagnitude;
end MappedBrakeEvaluation;
