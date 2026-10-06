within ModelicaAutomotive.Tests.SuspensionRigidBody;
model FullBodyTrim "Straight constant-speed full-body trim"
  ModelicaAutomotive.VehicleDynamics.RigidBody.FullBody vehicle(
    massProperties(mass=1000),
    initialState(velocityBody={10, 0, 0}));
  output Real position[3];
  output Real velocityBody[3];
  output Real quaternion[4];
  output Real quaternionNorm;
equation
  vehicle.forceBody = {0, 0, 1000 * 9.80665};
  vehicle.momentBody = {0, 0, 0};
  position = vehicle.positionWorld;
  velocityBody = vehicle.velocityBody;
  quaternion = vehicle.quaternionBodyToWorld;
  quaternionNorm = vehicle.quaternionNorm;
end FullBodyTrim;
