within ModelicaAerospace.Sensors;
block FirstOrderDelay "First-order approximation of sensor delay"
  parameter Real timeConstant(unit="s") = 0.1;
  parameter Real initialOutput = 0;
  ModelicaAerospace.Interfaces.RealInput u;
  ModelicaAerospace.Interfaces.RealOutput y;
protected
  Real state(start=initialOutput, fixed=true);
equation
  assert(timeConstant > 0, "Delay time constant must be positive");
  der(state) = (u - state) / timeConstant;
  y = state;
end FirstOrderDelay;
