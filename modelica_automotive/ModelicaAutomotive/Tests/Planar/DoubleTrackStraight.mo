within ModelicaAutomotive.Tests.Planar;
model DoubleTrackStraight "Symmetric longitudinal-force analytic motion"
  ModelicaAutomotive.VehicleDynamics.Planar.DoubleTrack vehicle(
    mass=1000,
    yawInertia=1800,
    initialState(longitudinalVelocity=10));
  output Real positionX;
  output Real longitudinalVelocity;
  output Real longitudinalAcceleration;
  output Real yawRate;
  output Real normalLoad[4];
equation
  vehicle.tireLongitudinalForce = {500, 500, 500, 500};
  vehicle.tireLateralForce = {0, 0, 0, 0};
  vehicle.steeringAngle = {0, 0, 0, 0};
  vehicle.externalLongitudinalForce = 0;
  vehicle.externalLateralForce = 0;
  vehicle.externalYawMoment = 0;
  positionX = vehicle.positionX;
  longitudinalVelocity = vehicle.longitudinalVelocity;
  longitudinalAcceleration = vehicle.longitudinalAcceleration;
  yawRate = vehicle.yawRate;
  normalLoad = vehicle.normalLoad;
end DoubleTrackStraight;
