within ModelicaAutomotive.Suspension;
block VerticalTire "Unilateral linear vertical tire contact"
  parameter Real stiffness(unit="N/m") = 200000;
  parameter Real damping(unit="N.s/m") = 1000;
  ModelicaAutomotive.Interfaces.RealInput roadHeight(unit="m");
  ModelicaAutomotive.Interfaces.RealInput wheelHeight(unit="m");
  ModelicaAutomotive.Interfaces.RealInput roadVelocity(unit="m/s");
  ModelicaAutomotive.Interfaces.RealInput wheelVelocity(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput normalForce(unit="N");
  ModelicaAutomotive.Interfaces.RealOutput deflection(unit="m");
equation
  assert(stiffness > 0, "stiffness must be positive");
  assert(damping >= 0, "damping must not be negative");
  deflection = roadHeight - wheelHeight;
  normalForce = max(
    0,
    stiffness * deflection + damping * (roadVelocity - wheelVelocity));
end VerticalTire;
