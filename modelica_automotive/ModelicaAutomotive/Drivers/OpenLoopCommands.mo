within ModelicaAutomotive.Drivers;
block OpenLoopCommands "Bound open-loop propulsion, brake, and steering commands"
  parameter ModelicaAutomotive.Types.Angle maximumSteeringAngle = 0.6;
  ModelicaAutomotive.Interfaces.RealInput propulsionRequest;
  ModelicaAutomotive.Interfaces.RealInput brakeRequest;
  ModelicaAutomotive.Interfaces.RealInput steeringRequest(unit="rad");
  ModelicaAutomotive.Interfaces.RealOutput propulsionCommand;
  ModelicaAutomotive.Interfaces.RealOutput brakeCommand;
  ModelicaAutomotive.Interfaces.RealOutput steeringCommand(unit="rad");
equation
  assert(maximumSteeringAngle > 0, "maximumSteeringAngle must be positive");
  propulsionCommand = min(max(propulsionRequest, 0), 1);
  brakeCommand = min(max(brakeRequest, 0), 1);
  steeringCommand = min(max(
    steeringRequest,
    -maximumSteeringAngle),
    maximumSteeringAngle);
end OpenLoopCommands;
