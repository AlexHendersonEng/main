within ModelicaAutomotive.Powertrain;
block TorqueDistributor "Configurable front/rear torque distribution"
  parameter Real frontFraction(min=0, max=1) = 0.5;
  ModelicaAutomotive.Interfaces.RealInput inputTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput frontTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput rearTorque(unit="N.m");
equation
  assert(frontFraction >= 0 and frontFraction <= 1,
    "frontFraction must be in [0, 1]");
  frontTorque = frontFraction * inputTorque;
  rearTorque = (1 - frontFraction) * inputTorque;
end TorqueDistributor;
