within ModelicaAutomotive.Tests.DriversSensorsControl;
model SlipControlValidation "ABS and traction control reduce excessive slip"
  ModelicaAutomotive.Control.AntiLockBraking absController(
    activationSlip=0.12,
    lockedSlip=0.3,
    minimumCommand=0.05);
  ModelicaAutomotive.Control.TractionControl tractionController(
    activationSlip=0.1,
    maximumSlip=0.25,
    minimumCommand=0.05,
    drivenWheelMask={1, 1, 0, 0});
  output Real uncontrolledBrakeSlip(start=0, fixed=true);
  output Real controlledBrakeSlip(start=0, fixed=true);
  output Real uncontrolledDriveSlip(start=0, fixed=true);
  output Real controlledDriveSlip(start=0, fixed=true);
  output Real brakeCommand;
  output Real propulsionCommand;
  output Real absActive;
  output Real tractionActive;
equation
  absController.brakeCommand = 1;
  absController.slipRatio =
    {controlledBrakeSlip, controlledBrakeSlip, controlledBrakeSlip,
      controlledBrakeSlip};
  tractionController.propulsionCommand = 1;
  tractionController.slipRatio =
    {controlledDriveSlip, controlledDriveSlip, 0, 0};
  der(uncontrolledBrakeSlip) = -2.5 - 5 * uncontrolledBrakeSlip;
  der(controlledBrakeSlip) =
    -2.5 * absController.wheelBrakeCommand[1] - 5 * controlledBrakeSlip;
  der(uncontrolledDriveSlip) = 2 - 5 * uncontrolledDriveSlip;
  der(controlledDriveSlip) =
    2 * tractionController.limitedPropulsionCommand - 5 * controlledDriveSlip;
  brakeCommand = absController.wheelBrakeCommand[1];
  propulsionCommand = tractionController.limitedPropulsionCommand;
  absActive = absController.active[1];
  tractionActive = tractionController.active;
end SlipControlValidation;
