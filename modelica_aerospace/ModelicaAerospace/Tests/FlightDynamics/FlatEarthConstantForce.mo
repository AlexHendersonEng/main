within ModelicaAerospace.Tests.FlightDynamics;
model FlatEarthConstantForce "Flat-Earth constant body force with gravity disabled"
  ModelicaAerospace.FlightDynamics.RigidBody.FlatEarth plant(
    massProperties(mass=4, inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 5]),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={10, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    gravity=0);
  output Real positionNED[3];
  output Real velocityBody[3];
  output Real accelerationBody[3];
equation
  plant.forceBody = {8, -4, 2};
  plant.momentBody = {0, 0, 0};
  positionNED = plant.positionNED;
  velocityBody = plant.velocityBody;
  accelerationBody = plant.accelerationBody;
end FlatEarthConstantForce;
