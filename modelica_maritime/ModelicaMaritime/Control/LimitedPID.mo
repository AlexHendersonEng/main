within ModelicaMaritime.Control;
block LimitedPID "Portable limited PID controller with anti-windup"
  parameter Real k = 1;
  parameter Real Ti(unit="s") = 1;
  parameter Real Td(unit="s") = 0.1;
  parameter Boolean derivativeEnabled = false;
  parameter Real outputMaximum = 1;
  parameter Real outputMinimum = -outputMaximum;
  parameter Real antiWindup(unit="1/s") = 5;
  parameter Real derivativeFilterTimeConstant(unit="s") = 0.05;
  parameter Real initialOutput = 0;
  ModelicaMaritime.Interfaces.RealInput setpoint;
  ModelicaMaritime.Interfaces.RealInput measurement;
  ModelicaMaritime.Interfaces.RealOutput command;
  ModelicaMaritime.Interfaces.RealOutput error;
protected
  Real integralState(start=initialOutput / k, fixed=true);
  Real filteredError(start=0, fixed=true);
  Real derivativeTerm;
  Real unlimitedCommand;
equation
  assert(k > 0, "PID gain must be positive");
  assert(Ti > 0, "PID integral time must be positive");
  assert(Td >= 0, "PID derivative time must be non-negative");
  assert(antiWindup >= 0, "PID anti-windup gain must be non-negative");
  assert(
    derivativeFilterTimeConstant > 0,
    "PID derivative filter time constant must be positive");
  assert(outputMaximum > outputMinimum, "PID output limits are invalid");
  error = setpoint - measurement;
  der(filteredError) = (error - filteredError) / derivativeFilterTimeConstant;
  derivativeTerm = if derivativeEnabled then
    k * Td * (error - filteredError) / derivativeFilterTimeConstant else 0;
  unlimitedCommand = k * (error + integralState) + derivativeTerm;
  command = min(max(unlimitedCommand, outputMinimum), outputMaximum);
  der(integralState) = error / Ti
    + antiWindup * (command - unlimitedCommand) / k;
end LimitedPID;
