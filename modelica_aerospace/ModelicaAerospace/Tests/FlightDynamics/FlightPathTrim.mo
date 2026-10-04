within ModelicaAerospace.Tests.FlightDynamics;
model FlightPathTrim "Level constant-speed flight-path equilibrium"
  ModelicaAerospace.FlightDynamics.PointMass.FlightPath plant(
    mass=5,
    gravity=9.80665,
    initialPositionNED={10, 20, -1000},
    initialSpeed=120,
    initialFlightPathAngle=0,
    initialGroundTrack=0.4);
  output Real positionNED[3];
  output Real velocityNED[3];
  output Real speed;
  output Real flightPathAngle;
  output Real groundTrack;
equation
  plant.forceVelocityAxes = {0, 5 * 9.80665, 0};
  positionNED = plant.positionNED;
  velocityNED = plant.velocityNED;
  speed = plant.speed;
  flightPathAngle = plant.flightPathAngle;
  groundTrack = plant.groundTrack;
end FlightPathTrim;
