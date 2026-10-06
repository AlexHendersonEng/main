within ModelicaAutomotive.Powertrain;
block FirstOrderMachine "First-order engine or motor with speed-dependent torque limit"
  parameter ModelicaAutomotive.Types.MachineParameters parameters;
  parameter Real initialCommand(min=-1, max=1) = 0;
  ModelicaAutomotive.Interfaces.RealInput command
    "Normalized command from -1 regenerative to +1 drive";
  ModelicaAutomotive.Interfaces.RealInput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput torque(unit="N.m");
  ModelicaAutomotive.Interfaces.RealOutput mechanicalPower(unit="W");
  ModelicaAutomotive.Interfaces.RealOutput sourcePower(unit="W")
    "Positive when energy is drawn from storage";
  ModelicaAutomotive.Interfaces.RealOutput commandState;
  ModelicaAutomotive.Interfaces.RealOutput speedLimitFactor;
protected
  Real commandStateInternal(start=initialCommand, fixed=true);
  Real limitedCommand;
equation
  assert(parameters.maximumDriveTorque >= 0,
    "maximumDriveTorque must not be negative");
  assert(parameters.maximumRegenerativeTorque >= 0,
    "maximumRegenerativeTorque must not be negative");
  assert(parameters.baseSpeed >= 0, "baseSpeed must not be negative");
  assert(parameters.maximumSpeed > parameters.baseSpeed,
    "maximumSpeed must exceed baseSpeed");
  assert(parameters.responseTime > 0, "responseTime must be positive");
  limitedCommand = min(max(command, -1), 1);
  der(commandStateInternal) =
    (limitedCommand - commandStateInternal) / parameters.responseTime;
  speedLimitFactor =
    if abs(angularVelocity) <= parameters.baseSpeed then 1
    elseif abs(angularVelocity) >= parameters.maximumSpeed then 0
    else (parameters.maximumSpeed - abs(angularVelocity))
      / (parameters.maximumSpeed - parameters.baseSpeed);
  torque =
    if commandStateInternal >= 0 then
      parameters.maximumDriveTorque * commandStateInternal * speedLimitFactor
    else parameters.maximumRegenerativeTorque * commandStateInternal;
  mechanicalPower = torque * angularVelocity;
  sourcePower =
    if mechanicalPower >= 0 then
      mechanicalPower / parameters.driveEfficiency
    else mechanicalPower * parameters.regenerativeEfficiency;
  commandState = commandStateInternal;
end FirstOrderMachine;
