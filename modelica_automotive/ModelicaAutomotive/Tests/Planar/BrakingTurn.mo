within ModelicaAutomotive.Tests.Planar;
model BrakingTurn "Combined braking and cornering double-track maneuver"
  ModelicaAutomotive.VehicleDynamics.Planar.DoubleTrack vehicle(
    mass=1500,
    yawInertia=2500,
    initialState(longitudinalVelocity=20));
  output Real longitudinalVelocity;
  output Real lateralVelocity;
  output Real yawRate;
  output Real yaw;
equation
  vehicle.tireLongitudinalForce = {-1000, -1000, -800, -800};
  vehicle.tireLateralForce = {1200, 1200, 900, 900};
  vehicle.steeringAngle = {0.04, 0.04, 0, 0};
  vehicle.externalLongitudinalForce = 0;
  vehicle.externalLateralForce = 0;
  vehicle.externalYawMoment = 0;
  longitudinalVelocity = vehicle.longitudinalVelocity;
  lateralVelocity = vehicle.lateralVelocity;
  yawRate = vehicle.yawRate;
  yaw = vehicle.yaw;
end BrakingTurn;
