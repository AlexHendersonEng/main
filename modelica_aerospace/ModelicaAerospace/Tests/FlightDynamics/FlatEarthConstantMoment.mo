within ModelicaAerospace.Tests.FlightDynamics;
model FlatEarthConstantMoment "Principal-axis constant-moment rigid-body case"
  ModelicaAerospace.FlightDynamics.RigidBody.FlatEarth plant(
    massProperties(mass=3, inertiaBody=[2, 0, 0; 0, 4, 0; 0, 0, 5]),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    gravity=0);
  output Real angularVelocity[3];
  output Real angularAcceleration[3];
  output Real quaternion[4];
  output Real quaternionNorm;
equation
  plant.forceBody = {0, 0, 0};
  plant.momentBody = {2, 0, 0};
  angularVelocity = plant.angularVelocityBody;
  angularAcceleration = plant.angularAccelerationBody;
  quaternion = plant.quaternionBodyToNED;
  quaternionNorm = plant.quaternionNorm;
end FlatEarthConstantMoment;
