within ModelicaAutomotive.Powertrain;
block IdealTorqueSource "Commanded positive-drive and regenerative torque source"
  parameter ModelicaAutomotive.Types.Torque maximumDriveTorque = 300;
  parameter ModelicaAutomotive.Types.Torque maximumRegenerativeTorque = 150;
  ModelicaAutomotive.Interfaces.RealInput command
    "Normalized command from -1 regenerative to +1 drive";
  ModelicaAutomotive.Interfaces.RealInput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput torque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput mechanicalPower(unit="W");
equation
  assert(maximumDriveTorque >= 0, "maximumDriveTorque must not be negative");
  assert(maximumRegenerativeTorque >= 0,
    "maximumRegenerativeTorque must not be negative");
  torque =
    if command >= 0 then maximumDriveTorque * min(command, 1)
    else maximumRegenerativeTorque * max(command, -1);
  mechanicalPower = torque * angularVelocity;
end IdealTorqueSource;
