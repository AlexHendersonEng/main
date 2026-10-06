within ModelicaMaritime.Actuators;
block CommandLimiter "Portable magnitude and slew-rate limited command"
  parameter Real minimum = -1;
  parameter Real maximum = 1;
  parameter Real risingRate(unit="1/s") = 1;
  parameter Real fallingRate(unit="1/s") = -risingRate;
  parameter Real derivativeTimeConstant(unit="s") = 0.001;
  parameter Real initialOutput = 0;
  ModelicaMaritime.Interfaces.RealInput rawCommand;
  ModelicaMaritime.Interfaces.RealOutput limitedCommand;
  ModelicaMaritime.Interfaces.RealOutput commandRate(unit="1/s");
protected
  Real commandState(start=initialOutput, fixed=true);
  Real demandedCommand;
  Real unconstrainedRate(unit="1/s");
equation
  assert(maximum > minimum, "Command bounds are invalid");
  assert(
    initialOutput >= minimum and initialOutput <= maximum,
    "Initial limited command is outside its bounds");
  assert(risingRate > 0, "Rising rate must be positive");
  assert(fallingRate < 0, "Falling rate must be negative");
  assert(derivativeTimeConstant > 0, "Limiter derivative time constant must be positive");
  demandedCommand = min(max(rawCommand, minimum), maximum);
  unconstrainedRate = (demandedCommand - commandState) / derivativeTimeConstant;
  commandRate = if commandState >= maximum and unconstrainedRate > 0 then 0
    else if commandState <= minimum and unconstrainedRate < 0 then 0
    else min(max(unconstrainedRate, fallingRate), risingRate);
  der(commandState) = commandRate;
  limitedCommand = min(max(commandState, minimum), maximum);
end CommandLimiter;
