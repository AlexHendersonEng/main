within ModelicaAutomotive.Control;
block TractionControl "Reduce propulsion command when driven-wheel slip is excessive"
  parameter Real activationSlip(min=0) = 0.1;
  parameter Real maximumSlip(min=0) = 0.25;
  parameter Real minimumCommand(min=0, max=1) = 0;
  parameter Real drivenWheelMask[4] = {1, 1, 0, 0}
    "Positive values select driven wheels";
  ModelicaAutomotive.Interfaces.RealInput propulsionCommand;
  ModelicaAutomotive.Interfaces.CornerInput slipRatio;
  ModelicaAutomotive.Interfaces.RealOutput limitedPropulsionCommand;
  ModelicaAutomotive.Interfaces.RealOutput active;
  ModelicaAutomotive.Interfaces.RealOutput maximumDrivenSlip;
  ModelicaAutomotive.Interfaces.RealOutput modulation;
protected
  Real selectedSlip[4];
equation
  assert(maximumSlip > activationSlip,
    "maximumSlip must exceed activationSlip");
  assert(minimumCommand >= 0 and minimumCommand <= 1,
    "minimumCommand must be in [0, 1]");
  assert(sum(drivenWheelMask) > 0,
    "drivenWheelMask must select at least one wheel");
  for corner in 1:4 loop
    selectedSlip[corner] =
      if drivenWheelMask[corner] > 0 then slipRatio[corner] else 0;
  end for;
  maximumDrivenSlip = max(
    max(selectedSlip[1], selectedSlip[2]),
    max(selectedSlip[3], selectedSlip[4]));
  modulation = min(max(
    (maximumDrivenSlip - activationSlip)
    / (maximumSlip - activationSlip),
    0),
    1);
  active = if modulation > 0 then 1 else 0;
  limitedPropulsionCommand = min(max(propulsionCommand, 0), 1)
    * max(1 - modulation, minimumCommand);
end TractionControl;
