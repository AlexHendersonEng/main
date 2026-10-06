within ModelicaAutomotive.Drivers;
block SpeedController "PI speed controller with command splitting and anti-windup"
  parameter Real proportionalGain(unit="s/m") = 0.2;
  parameter Real integralGain(unit="1/m") = 0.05;
  parameter Real antiWindupGain(unit="1/s") = 2;
  parameter Real disabledResetTime(unit="s") = 0.5;
  parameter Real initialIntegral = 0;
  ModelicaAutomotive.Interfaces.RealInput targetSpeed(unit="m/s");
  ModelicaAutomotive.Interfaces.RealInput measuredSpeed(unit="m/s");
  ModelicaAutomotive.Interfaces.RealInput enable
    "Enabled when greater than or equal to 0.5";
  ModelicaAutomotive.Interfaces.RealOutput propulsionCommand;
  ModelicaAutomotive.Interfaces.RealOutput brakeCommand;
  ModelicaAutomotive.Interfaces.RealOutput controlEffort;
  ModelicaAutomotive.Interfaces.RealOutput speedError(unit="m/s");
  ModelicaAutomotive.Interfaces.RealOutput integralState;
protected
  Real integralStateInternal(start=initialIntegral, fixed=true);
  Real unsaturatedEffort;
equation
  assert(proportionalGain >= 0, "proportionalGain must not be negative");
  assert(integralGain >= 0, "integralGain must not be negative");
  assert(antiWindupGain > 0, "antiWindupGain must be positive");
  assert(disabledResetTime > 0, "disabledResetTime must be positive");
  speedError = targetSpeed - measuredSpeed;
  unsaturatedEffort = proportionalGain * speedError + integralStateInternal;
  controlEffort =
    if enable >= 0.5 then min(max(unsaturatedEffort, -1), 1) else 0;
  der(integralStateInternal) =
    if enable >= 0.5 then
      integralGain * speedError
      + antiWindupGain * (controlEffort - unsaturatedEffort)
    else -integralStateInternal / disabledResetTime;
  propulsionCommand = max(controlEffort, 0);
  brakeCommand = max(-controlEffort, 0);
  integralState = integralStateInternal;
end SpeedController;
