within ModelicaAutomotive.Scenarios;
record ActorState "Road-actor pose and velocity state"
  Real positionWorld[3](each unit="m") = {0, 0, 0};
  Real velocityWorld[3](each unit="m/s") = {0, 0, 0};
  Real yaw(unit="rad") = 0;
  Real yawRate(unit="rad/s") = 0;
end ActorState;
