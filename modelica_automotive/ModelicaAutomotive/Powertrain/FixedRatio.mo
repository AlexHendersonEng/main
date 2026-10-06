within ModelicaAutomotive.Powertrain;
block FixedRatio "Ideal fixed-ratio transmission with forward efficiency"
  parameter Real ratio(min=ModelicaAutomotive.Constants.small) = 4;
  parameter Real efficiency(min=ModelicaAutomotive.Constants.small, max=1) = 0.97;
  ModelicaAutomotive.Interfaces.RealInput inputTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealInput outputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput inputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput outputTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput inputPower(unit="W");
  ModelicaAutomotive.Interfaces.RealOutput outputPower(unit="W");
equation
  assert(ratio > 0, "ratio must be positive");
  inputAngularVelocity = ratio * outputAngularVelocity;
  outputTorque =
    if inputTorque * inputAngularVelocity >= 0 then
      inputTorque * ratio * efficiency
    else inputTorque * ratio / efficiency;
  inputPower = inputTorque * inputAngularVelocity;
  outputPower = outputTorque * outputAngularVelocity;
end FixedRatio;
