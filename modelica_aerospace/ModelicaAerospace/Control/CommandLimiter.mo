within ModelicaAerospace.Control;
block CommandLimiter "MSL slew-rate and magnitude limited command"
  parameter Real minimum = -1;
  parameter Real maximum = 1;
  parameter Real risingRate(unit="1/s") = 1;
  parameter Real fallingRate(unit="1/s") = -risingRate;
  parameter Real derivativeTimeConstant(unit="s") = 0.001;
  parameter Real initialOutput = 0;
  ModelicaAerospace.Interfaces.RealInput rawCommand;
  ModelicaAerospace.Interfaces.RealOutput limitedCommand;
protected
  Modelica.Blocks.Nonlinear.SlewRateLimiter slew(
    Rising=risingRate,
    Falling=fallingRate,
    Td=derivativeTimeConstant,
    initType=Modelica.Blocks.Types.Init.InitialOutput,
    y_start=initialOutput);
  Modelica.Blocks.Nonlinear.Limiter limiter(
    uMin=minimum,
    uMax=maximum);
equation
  limiter.u = rawCommand;
  slew.u = limiter.y;
  limitedCommand = slew.y;
end CommandLimiter;
