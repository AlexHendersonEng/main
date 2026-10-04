within ModelicaAerospace.Tests.FlightDynamics;
model SphericalBallistic "Unforced central-gravity ballistic trajectory"
  ModelicaAerospace.FlightDynamics.RigidBody.SphericalEarth plant(
    massProperties(mass=100, inertiaBody=[20, 0, 0; 0, 30, 0; 0, 0, 40]),
    initialState(
      positionECEF={7000000, 0, 0},
      velocityECEF={0, 7500, 0},
      quaternionBodyToECEF={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    earthRotationRate=0);
  output Real positionECEF[3];
  output Real velocityECEF[3];
  output Real energy;
  output Real angularMomentum[3];
  output Real quaternionNorm;
equation
  plant.forceBody = {0, 0, 0};
  plant.momentBody = {0, 0, 0};
  positionECEF = plant.positionECEF;
  velocityECEF = plant.velocityECEF;
  energy = plant.specificMechanicalEnergy;
  angularMomentum = plant.specificAngularMomentum;
  quaternionNorm = plant.quaternionNorm;
end SphericalBallistic;
