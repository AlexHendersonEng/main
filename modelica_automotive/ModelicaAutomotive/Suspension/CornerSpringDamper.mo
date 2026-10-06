within ModelicaAutomotive.Suspension;
block CornerSpringDamper "Linear corner suspension force on the sprung body"
  parameter Real stiffness(unit="N/m") = 30000;
  parameter Real damping(unit="N.s/m") = 3000;
  parameter ModelicaAutomotive.Types.Force preload = 0;
  ModelicaAutomotive.Interfaces.RealInput compression(unit="m")
    "Positive when the wheel/road moves upward relative to the body";
  ModelicaAutomotive.Interfaces.RealInput compressionRate(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput forceOnBody(unit="N");
equation
  assert(stiffness >= 0, "stiffness must not be negative");
  assert(damping >= 0, "damping must not be negative");
  forceOnBody = preload + stiffness * compression + damping * compressionRate;
end CornerSpringDamper;
