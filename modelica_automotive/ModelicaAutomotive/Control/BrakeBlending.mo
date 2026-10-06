within ModelicaAutomotive.Control;
block BrakeBlending "Allocate a normalized brake request between regeneration and friction"
  parameter Real maximumRegenerativeFraction(min=0, max=1) = 0.7;
  ModelicaAutomotive.Interfaces.RealInput brakeCommand;
  ModelicaAutomotive.Interfaces.RealInput regenerativeAvailability;
  ModelicaAutomotive.Interfaces.RealOutput regenerativeCommand
    "Negative normalized machine command";
  ModelicaAutomotive.Interfaces.RealOutput frictionBrakeCommand;
  ModelicaAutomotive.Interfaces.RealOutput achievedBrakeCommand;
protected
  Real boundedCommand;
  Real regenerativeMagnitude;
equation
  assert(maximumRegenerativeFraction >= 0
      and maximumRegenerativeFraction <= 1,
    "maximumRegenerativeFraction must be in [0, 1]");
  boundedCommand = min(max(brakeCommand, 0), 1);
  regenerativeMagnitude = min(
    boundedCommand * maximumRegenerativeFraction,
    min(max(regenerativeAvailability, 0), 1));
  regenerativeCommand = -regenerativeMagnitude;
  frictionBrakeCommand = boundedCommand - regenerativeMagnitude;
  achievedBrakeCommand = -regenerativeCommand + frictionBrakeCommand;
end BrakeBlending;
