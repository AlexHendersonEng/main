within ModelicaAutomotive.Steering;
block FirstOrderSteering "First-order rate- and magnitude-limited steering response"
  parameter Real timeConstant(unit="s") = 0.1;
  parameter ModelicaAutomotive.Types.Angle maximumAngle = 0.6;
  parameter ModelicaAutomotive.Types.AngularVelocity maximumRate = 5;
  parameter ModelicaAutomotive.Types.Angle initialAngle = 0;
  ModelicaAutomotive.Interfaces.RealInput command(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput angle(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput rate(unit="rad/s");
protected
  Real angleState(start=initialAngle, fixed=true, unit="rad");
  Real unconstrainedRate(unit="rad/s");
equation
  assert(timeConstant > 0, "timeConstant must be positive");
  assert(maximumAngle > 0, "maximumAngle must be positive");
  assert(maximumRate > 0, "maximumRate must be positive");
  unconstrainedRate =
    (min(max(command, -maximumAngle), maximumAngle) - angleState) / timeConstant;
  rate = min(max(unconstrainedRate, -maximumRate), maximumRate);
  der(angleState) = rate;
  angle = angleState;
end FirstOrderSteering;
