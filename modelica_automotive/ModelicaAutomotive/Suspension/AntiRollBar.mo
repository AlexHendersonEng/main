within ModelicaAutomotive.Suspension;
block AntiRollBar "Equal-and-opposite body forces from axle compression difference"
  parameter Real stiffness(unit="N/m") = 10000;
  ModelicaAutomotive.Interfaces.RealInput leftCompression(unit="m");
  ModelicaAutomotive.Interfaces.RealInput rightCompression(unit="m");
  ModelicaAutomotive.Interfaces.RealOutput leftForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput rightForce(unit="N");
equation
  assert(stiffness >= 0, "stiffness must not be negative");
  leftForce = stiffness * (leftCompression - rightCompression);
  rightForce = -leftForce;
end AntiRollBar;
