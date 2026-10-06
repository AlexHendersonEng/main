within ModelicaAutomotive.Control;
block AntiLockBraking "Four-wheel brake modulation from negative slip ratio"
  parameter Real activationSlip(min=0) = 0.12;
  parameter Real lockedSlip(min=0) = 0.3;
  parameter Real minimumCommand(min=0, max=1) = 0.05;
  ModelicaAutomotive.Interfaces.RealInput brakeCommand;
  ModelicaAutomotive.Interfaces.CornerInput slipRatio;
  ModelicaAutomotive.Interfaces.CornerOutput wheelBrakeCommand;
  ModelicaAutomotive.Interfaces.CornerOutput active;
  ModelicaAutomotive.Interfaces.CornerOutput modulation;
protected
  Real boundedCommand;
equation
  assert(lockedSlip > activationSlip,
    "lockedSlip must exceed activationSlip");
  assert(minimumCommand >= 0 and minimumCommand <= 1,
    "minimumCommand must be in [0, 1]");
  boundedCommand = min(max(brakeCommand, 0), 1);
  for corner in 1:4 loop
    modulation[corner] = min(max(
      (-slipRatio[corner] - activationSlip)
      / (lockedSlip - activationSlip),
      0),
      1);
    active[corner] = if modulation[corner] > 0 then 1 else 0;
    wheelBrakeCommand[corner] = boundedCommand
      * max(1 - modulation[corner], minimumCommand);
  end for;
end AntiLockBraking;
