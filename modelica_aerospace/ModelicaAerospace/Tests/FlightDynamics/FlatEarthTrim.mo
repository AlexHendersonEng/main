within ModelicaAerospace.Tests.FlightDynamics;
model FlatEarthTrim "Level flat-Earth rigid-body equilibrium"
  ModelicaAerospace.FlightDynamics.RigidBody.FlatEarth plant(
    massProperties(mass=10, inertiaBody=[4, 0, 0; 0, 5, 0; 0, 0, 6]),
    initialState(
      positionNED={0, 0, -1000},
      velocityBody={80, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}));
  output Real positionNED[3];
  output Real velocityBody[3];
  output Real quaternion[4];
  output Real angularVelocity[3];
  output Real loadFactor[3];
equation
  plant.forceBody = {0, 0, -10 * 9.80665};
  plant.momentBody = {0, 0, 0};
  positionNED = plant.positionNED;
  velocityBody = plant.velocityBody;
  quaternion = plant.quaternionBodyToNED;
  angularVelocity = plant.angularVelocityBody;
  loadFactor = plant.loadFactorBody;
end FlatEarthTrim;
