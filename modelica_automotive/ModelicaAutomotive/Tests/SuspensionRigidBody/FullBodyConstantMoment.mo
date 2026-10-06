within ModelicaAutomotive.Tests.SuspensionRigidBody;
model FullBodyConstantMoment "Full-body constant principal-axis roll moment"
  ModelicaAutomotive.VehicleDynamics.RigidBody.FullBody vehicle(
    massProperties(
      mass=1000,
      inertiaBody=[600, 0, 0; 0, 1800, 0; 0, 0, 2000]));
  output Real angularVelocity[3];
  output Real angularAcceleration[3];
  output Real quaternion[4];
  output Real quaternionNorm;
equation
  vehicle.forceBody = {0, 0, 1000 * 9.80665};
  vehicle.momentBody = {600, 0, 0};
  angularVelocity = vehicle.angularVelocityBody;
  angularAcceleration = vehicle.angularAccelerationBody;
  quaternion = vehicle.quaternionBodyToWorld;
  quaternionNorm = vehicle.quaternionNorm;
end FullBodyConstantMoment;
