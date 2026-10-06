within ModelicaAutomotive.Powertrain;
block SelectableTransmission "Discrete selectable gear ratios"
  parameter Real ratios[:] = {10, 6, 4};
  parameter Real efficiency(min=ModelicaAutomotive.Constants.small, max=1) = 0.97;
  ModelicaAutomotive.Interfaces.RealInput gearCommand
    "One-based gear number";
  ModelicaAutomotive.Interfaces.RealInput inputTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealInput outputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput selectedRatio;
  ModelicaAutomotive.Interfaces.RealOutput inputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput outputTorque(unit="N.m");
equation
  assert(min(ratios) > 0, "All transmission ratios must be positive");
  assert(
    gearCommand >= 0.5 and gearCommand < size(ratios, 1) + 0.5,
    "gearCommand must select an available one-based gear");
  selectedRatio = sum(
    if gearCommand >= gearIndex - 0.5
        and gearCommand < gearIndex + 0.5 then ratios[gearIndex] else 0
    for gearIndex in 1:size(ratios, 1));
  inputAngularVelocity = selectedRatio * outputAngularVelocity;
  outputTorque =
    if inputTorque * inputAngularVelocity >= 0 then
      inputTorque * selectedRatio * efficiency
    else inputTorque * selectedRatio / efficiency;
end SelectableTransmission;
