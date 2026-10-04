within ModelicaAerospace.Tests.FlightDynamics;
model SphericalLocalLimit "Short-time spherical-Earth local NED limiting case"
  parameter Real radius = 6378137;
  parameter Real gravity = 9.80665;
  ModelicaAerospace.FlightDynamics.RigidBody.SphericalEarth plant(
    massProperties(mass=2, inertiaBody=[1, 0, 0; 0, 1, 0; 0, 0, 1]),
    initialState(
      positionECEF={radius, 0, 0},
      velocityECEF={0, 0, 50},
      quaternionBodyToECEF={0.7071067811865476, 0, -0.7071067811865476, 0},
      angularVelocityBody={0, 0, 0}),
    gravitationalParameter=gravity * radius * radius,
    earthRotationRate=0);
  output Real positionECEF[3];
  output Real velocityECEF[3];
  output Real velocityBody[3];
  output Real altitude;
equation
  plant.forceBody = {4, 0, 0};
  plant.momentBody = {0, 0, 0};
  positionECEF = plant.positionECEF;
  velocityECEF = plant.velocityECEF;
  velocityBody = plant.velocityBody;
  altitude = plant.altitude;
end SphericalLocalLimit;
