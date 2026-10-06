within ModelicaAutomotive.Powertrain;
block OpenDifferential "Equal-torque open differential with final drive"
  parameter Real finalDriveRatio(min=ModelicaAutomotive.Constants.small) = 3.5;
  parameter Real efficiency(min=ModelicaAutomotive.Constants.small, max=1) = 0.97;
  ModelicaAutomotive.Interfaces.RealInput inputTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealInput leftAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealInput rightAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput inputAngularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput leftTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput rightTorque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput inputPower(unit="W");
  ModelicaAutomotive.Interfaces.RealOutput outputPower(unit="W");
equation
  assert(finalDriveRatio > 0, "finalDriveRatio must be positive");
  inputAngularVelocity =
    finalDriveRatio * (leftAngularVelocity + rightAngularVelocity) / 2;
  leftTorque =
    if inputTorque * inputAngularVelocity >= 0 then
      inputTorque * finalDriveRatio * efficiency / 2
    else inputTorque * finalDriveRatio / (2 * efficiency);
  rightTorque = leftTorque;
  inputPower = inputTorque * inputAngularVelocity;
  outputPower =
    leftTorque * leftAngularVelocity + rightTorque * rightAngularVelocity;
end OpenDifferential;
