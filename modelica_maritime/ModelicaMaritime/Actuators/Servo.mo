within ModelicaMaritime.Actuators;
block Servo "Biased, deadbanded, limited servo with explicit failure position"
  parameter Real minimumPosition = -1;
  parameter Real maximumPosition = 1;
  parameter Real risingRate(unit="1/s") = 1;
  parameter Real fallingRate(unit="1/s") = -risingRate;
  parameter Real deadband(min=0) = 0;
  parameter Real failurePosition = 0;
  parameter Real initialPosition = 0;
  ModelicaMaritime.Interfaces.RealInput command;
  ModelicaMaritime.Interfaces.RealInput bias;
  input Boolean failed;
  ModelicaMaritime.Interfaces.RealOutput position;
  ModelicaMaritime.Interfaces.RealOutput positionRate(unit="1/s");
protected
  Real selectedCommand;
  ModelicaMaritime.Actuators.CommandLimiter limiter(
    minimum=minimumPosition,
    maximum=maximumPosition,
    risingRate=risingRate,
    fallingRate=fallingRate,
    initialOutput=initialPosition);
equation
  selectedCommand = if failed then failurePosition
    else if abs(command + bias) <= deadband then 0
    else command + bias;
  limiter.rawCommand = selectedCommand;
  position = limiter.limitedCommand;
  positionRate = limiter.commandRate;
end Servo;
