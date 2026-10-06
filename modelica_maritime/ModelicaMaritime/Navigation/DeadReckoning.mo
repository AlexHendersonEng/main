within ModelicaMaritime.Navigation;
block DeadReckoning "Integrate NED velocity and yaw rate"
  parameter ModelicaMaritime.Types.Vector3 initialPositionNED = {0, 0, 0};
  parameter ModelicaMaritime.Types.Angle initialHeading = 0;
  ModelicaMaritime.Interfaces.Vector3Input velocityNED(each unit="m/s");
  ModelicaMaritime.Interfaces.RealInput yawRate(unit="rad/s");
  ModelicaMaritime.Interfaces.Vector3Output estimatedPositionNED(each unit="m");
  ModelicaMaritime.Interfaces.RealOutput estimatedHeading(unit="rad");
protected
  Real positionState[3](start=initialPositionNED, each fixed=true);
  Real headingState(start=initialHeading, fixed=true);
equation
  der(positionState) = velocityNED;
  der(headingState) = yawRate;
  estimatedPositionNED = positionState;
  estimatedHeading = ModelicaMaritime.Mathematics.wrapHeading(headingState);
end DeadReckoning;
