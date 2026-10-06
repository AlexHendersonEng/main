within ModelicaMaritime.Propulsion;
block FirstOrderShaft "First-order reversible shaft or motor speed"
  parameter Real timeConstant(unit="s") = 1;
  parameter Real maximumForwardRate(unit="1/s") = 1;
  parameter Real maximumReverseRate(unit="1/s") = maximumForwardRate;
  parameter Real initialRate(unit="1/s") = 0;
  ModelicaMaritime.Interfaces.RealInput command;
  input Boolean enabled;
  input Boolean failed;
  ModelicaMaritime.Interfaces.RealOutput shaftRate(unit="1/s");
  ModelicaMaritime.Interfaces.RealOutput shaftAcceleration(unit="1/s2");
protected
  Real rateState(start=initialRate, fixed=true, unit="1/s");
  Real limitedCommand;
  Real demandedRate(unit="1/s");
equation
  assert(timeConstant > 0, "Shaft time constant must be positive");
  assert(maximumForwardRate >= 0, "Maximum forward rate must be non-negative");
  assert(maximumReverseRate >= 0, "Maximum reverse rate must be non-negative");
  limitedCommand = min(max(command, -1), 1);
  demandedRate = if enabled and not failed then
    if limitedCommand >= 0 then limitedCommand * maximumForwardRate
    else limitedCommand * maximumReverseRate
    else 0;
  shaftAcceleration = (demandedRate - rateState) / timeConstant;
  der(rateState) = shaftAcceleration;
  shaftRate = rateState;
end FirstOrderShaft;
