within ModelicaAutomotive.Tests.SuspensionRigidBody;
model PlanarLimit "Full-body and planar straight-line limiting case"
  ModelicaAutomotive.VehicleDynamics.RigidBody.FullBody fullBody(
    massProperties(mass=1000),
    initialState(velocityBody={10, 0, 0}));
  ModelicaAutomotive.VehicleDynamics.Planar.DoubleTrack planar(
    mass=1000,
    initialState(longitudinalVelocity=10));
  output Real positionDifference;
  output Real velocityDifference;
equation
  fullBody.forceBody = {2000, 0, 1000 * 9.80665};
  fullBody.momentBody = {0, 0, 0};
  planar.tireLongitudinalForce = {500, 500, 500, 500};
  planar.tireLateralForce = {0, 0, 0, 0};
  planar.steeringAngle = {0, 0, 0, 0};
  planar.externalLongitudinalForce = 0;
  planar.externalLateralForce = 0;
  planar.externalYawMoment = 0;
  positionDifference = fullBody.positionWorld[1] - planar.positionX;
  velocityDifference = fullBody.velocityBody[1] - planar.longitudinalVelocity;
end PlanarLimit;
