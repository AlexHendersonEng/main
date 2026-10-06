within ModelicaMaritime.Navigation;
block PositionFusion "Complementary NED position propagation and correction"
  parameter Real bandwidth(unit="1/s") = 0.2;
  parameter ModelicaMaritime.Types.Vector3 initialPositionNED = {0, 0, 0};
  ModelicaMaritime.Interfaces.Vector3Input velocityNED(each unit="m/s");
  ModelicaMaritime.Interfaces.Vector3Input absolutePositionNED(each unit="m");
  ModelicaMaritime.Interfaces.Vector3Output estimatedPositionNED(each unit="m");
protected
  Real positionState[3](start=initialPositionNED, each fixed=true);
equation
  assert(bandwidth > 0, "Position-fusion bandwidth must be positive");
  der(positionState) = velocityNED
    + bandwidth * (absolutePositionNED - positionState);
  estimatedPositionNED = positionState;
end PositionFusion;
