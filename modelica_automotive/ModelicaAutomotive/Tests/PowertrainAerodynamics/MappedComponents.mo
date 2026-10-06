within ModelicaAutomotive.Tests.PowertrainAerodynamics;
model MappedComponents "Evaluate MSL table-mapped torque and aerodynamic coefficients"
  ModelicaAutomotive.Powertrain.MappedTorqueSource machine;
  ModelicaAutomotive.Aerodynamics.MappedCoefficients aerodynamics;
  output Real torque;
  output Real dragCoefficient;
  output Real sideCoefficient;
  output Real liftCoefficient;
  output Real dragIntegral(start=0, fixed=true);
  output Real sideIntegral(start=0, fixed=true);
  output Real liftIntegral(start=0, fixed=true);
equation
  machine.command = 0.8;
  machine.angularVelocity = 450;
  aerodynamics.speed = 45;
  torque = machine.torque;
  dragCoefficient = aerodynamics.dragCoefficient;
  sideCoefficient = aerodynamics.sideCoefficient;
  liftCoefficient = aerodynamics.liftCoefficient;
  der(dragIntegral) = aerodynamics.dragCoefficient;
  der(sideIntegral) = aerodynamics.sideCoefficient;
  der(liftIntegral) = aerodynamics.liftCoefficient;
end MappedComponents;
