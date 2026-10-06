within ModelicaAutomotive.Powertrain;
block DriveshaftCompliance "Torsional spring-damper driveline compliance"
  parameter Real stiffness(unit="N.m/rad") = 10000;
  parameter Real damping(unit="N.m.s/rad") = 100;
  ModelicaAutomotive.Interfaces.RealInput inputAngle(unit="rad");
  ModelicaAutomotive.Interfaces.RealInput outputAngle(unit="rad");
  ModelicaAutomotive.Interfaces.RealInput inputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealInput outputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput transmittedTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput storedEnergy(unit="J");
  ModelicaAutomotive.Interfaces.RealOutput dissipatedPower(unit="W");
protected
  Real twist(unit="rad");
  Real slipSpeed(unit="rad/s");
equation
  assert(stiffness >= 0, "stiffness must not be negative");
  assert(damping >= 0, "damping must not be negative");
  twist = inputAngle - outputAngle;
  slipSpeed = inputAngularVelocity - outputAngularVelocity;
  transmittedTorque = stiffness * twist + damping * slipSpeed;
  storedEnergy = 0.5 * stiffness * twist * twist;
  dissipatedPower = damping * slipSpeed * slipSpeed;
end DriveshaftCompliance;
