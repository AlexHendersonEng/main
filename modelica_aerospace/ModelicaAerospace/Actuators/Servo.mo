within ModelicaAerospace.Actuators;
block Servo "First-order position- and rate-limited actuator"
  parameter Real timeConstant(unit="s") = 0.1;
  parameter Real minimumPosition = -1;
  parameter Real maximumPosition = 1;
  parameter Real maximumRate(unit="1/s") = 1;
  parameter Real deadband = 0;
  parameter Real failurePosition = 0;
  parameter Real initialPosition = 0;
  ModelicaAerospace.Interfaces.RealInput command;
  ModelicaAerospace.Interfaces.RealInput bias;
  input Boolean failed;
  ModelicaAerospace.Interfaces.RealOutput position;
  ModelicaAerospace.Interfaces.RealOutput positionRate(unit="1/s");
protected
  Real positionState(start=initialPosition, fixed=true);
  Real biasedCommand;
  Real demandedPosition;
  Real unconstrainedRate;
equation
  assert(timeConstant > 0, "Actuator time constant must be positive");
  assert(maximumPosition > minimumPosition, "Actuator position bounds are invalid");
  assert(maximumRate > 0, "Actuator maximum rate must be positive");
  assert(deadband >= 0, "Actuator deadband must be non-negative");
  biasedCommand = command + bias;
  demandedPosition = if failed then failurePosition else
    ModelicaAerospace.Mathematics.clamp(
      if abs(biasedCommand) <= deadband then 0 else biasedCommand,
      minimumPosition,
      maximumPosition);
  unconstrainedRate = (demandedPosition - positionState) / timeConstant;
  positionRate = if positionState >= maximumPosition and unconstrainedRate > 0 then 0
    else if positionState <= minimumPosition and unconstrainedRate < 0 then 0
    else ModelicaAerospace.Mathematics.clamp(
      unconstrainedRate,
      -maximumRate,
      maximumRate);
  der(positionState) = positionRate;
  position = ModelicaAerospace.Mathematics.clamp(
    positionState,
    minimumPosition,
    maximumPosition);
end Servo;
