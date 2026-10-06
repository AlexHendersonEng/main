within ModelicaAutomotive.Control;
block YawStabilityControl "Direct yaw-moment feedback with deadband and saturation"
  parameter Real proportionalGain(unit="N.m.s/rad") = 5000;
  parameter ModelicaAutomotive.Types.AngularVelocity deadband = 0.01;
  parameter ModelicaAutomotive.Types.Torque maximumYawMoment = 3000;
  ModelicaAutomotive.Interfaces.RealInput desiredYawRate(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealInput measuredYawRate(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput yawMomentCommand(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput yawRateError(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput active;
protected
  Real correctedError(unit="rad/s");
equation
  assert(proportionalGain >= 0, "proportionalGain must not be negative");
  assert(deadband >= 0, "deadband must not be negative");
  assert(maximumYawMoment >= 0, "maximumYawMoment must not be negative");
  yawRateError = desiredYawRate - measuredYawRate;
  correctedError =
    if yawRateError > deadband then yawRateError - deadband
    elseif yawRateError < -deadband then yawRateError + deadband
    else 0;
  yawMomentCommand = min(max(
    proportionalGain * correctedError,
    -maximumYawMoment),
    maximumYawMoment);
  active = if abs(correctedError) > 0 then 1 else 0;
end YawStabilityControl;
