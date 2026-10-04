within ModelicaAerospace.Tests.FlightDynamics;
model PointMassConstantForce "Cartesian point mass under constant acceleration"
  ModelicaAerospace.FlightDynamics.PointMass.Cartesian plant(
    mass=2,
    gravityNED={0, 0, 0},
    initialState(positionNED={1, 2, 3}, velocityNED={4, -2, 1}));
  output Real positionNED[3];
  output Real velocityNED[3];
  output Real accelerationNED[3];
equation
  plant.forceNED = {6, 4, -2};
  positionNED = plant.positionNED;
  velocityNED = plant.velocityNED;
  accelerationNED = plant.accelerationNED;
end PointMassConstantForce;
