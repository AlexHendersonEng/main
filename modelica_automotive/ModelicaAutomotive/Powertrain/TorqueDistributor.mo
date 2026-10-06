within ModelicaAutomotive.Powertrain;
block TorqueDistributor "Configurable front/rear torque distribution"
  parameter Real frontFraction(min=0, max=1) = 0.5;
  ModelicaAutomotive.Interfaces.RealInput inputTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput frontTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput rearTorque(unit="N.m");
equation
  frontTorque = frontFraction * inputTorque;
  rearTorque = (1 - frontFraction) * inputTorque;
end TorqueDistributor;
