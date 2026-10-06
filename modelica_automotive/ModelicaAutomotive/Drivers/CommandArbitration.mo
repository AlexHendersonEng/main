within ModelicaAutomotive.Drivers;
block CommandArbitration "Blend manual and automated commands with brake priority"
  parameter ModelicaAutomotive.Types.Angle maximumSteeringAngle = 0.6;
  ModelicaAutomotive.Interfaces.RealInput manualPropulsion;
  ModelicaAutomotive.Interfaces.RealInput manualBrake;
  ModelicaAutomotive.Interfaces.RealInput manualSteering(unit="rad");
  ModelicaAutomotive.Interfaces.RealInput automatedPropulsion;
  ModelicaAutomotive.Interfaces.RealInput automatedBrake;
  ModelicaAutomotive.Interfaces.RealInput automatedSteering(unit="rad");
  ModelicaAutomotive.Interfaces.RealInput automationBlend
    "Zero selects manual commands and one selects automated commands";
  ModelicaAutomotive.Interfaces.RealInput emergencyBrake;
  ModelicaAutomotive.Interfaces.RealOutput propulsionCommand;
  ModelicaAutomotive.Interfaces.RealOutput brakeCommand;
  ModelicaAutomotive.Interfaces.RealOutput steeringCommand(unit="rad");
protected
  Real blend;
  Real blendedPropulsion;
equation
  assert(maximumSteeringAngle > 0, "maximumSteeringAngle must be positive");
  blend = min(max(automationBlend, 0), 1);
  brakeCommand = max(
    min(max((1 - blend) * manualBrake + blend * automatedBrake, 0), 1),
    min(max(emergencyBrake, 0), 1));
  blendedPropulsion = min(max(
    (1 - blend) * manualPropulsion + blend * automatedPropulsion,
    0),
    1);
  propulsionCommand = blendedPropulsion * (1 - brakeCommand);
  steeringCommand = min(max(
    (1 - blend) * manualSteering + blend * automatedSteering,
    -maximumSteeringAngle),
    maximumSteeringAngle);
end CommandArbitration;
