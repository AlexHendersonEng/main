within ModelicaAutomotive.Tests.SuspensionRigidBody;
model FullBodyConstantForce "Full-body constant forward acceleration"
  ModelicaAutomotive.VehicleDynamics.RigidBody.FullBody vehicle(
    massProperties(mass=1000),
    initialState(velocityBody={10, 0, 0}));
  output Real position[3];
  output Real velocityBody[3];
  output Real accelerationBody[3];
equation
  vehicle.forceBody = {2000, 0, 1000 * 9.80665};
  vehicle.momentBody = {0, 0, 0};
  position = vehicle.positionWorld;
  velocityBody = vehicle.velocityBody;
  accelerationBody = vehicle.accelerationBody;
end FullBodyConstantForce;
