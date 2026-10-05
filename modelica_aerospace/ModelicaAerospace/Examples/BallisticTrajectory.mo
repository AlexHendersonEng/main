within ModelicaAerospace.Examples;
model BallisticTrajectory "Analytic flat-Earth ballistic trajectory"
  ModelicaAerospace.FlightDynamics.PointMass.Cartesian vehicle(
    mass=100,
    gravityNED={0, 0, 9.80665},
    initialState(
      positionNED={0, 0, 0},
      velocityNED={50, 0, -100}));
  output Real positionNED[3](each unit="m");
  output Real velocityNED[3](each unit="m/s");
  output Real specificMechanicalEnergy(unit="m2/s2");
equation
  vehicle.forceNED = {0, 0, 0};
  positionNED = vehicle.positionNED;
  velocityNED = vehicle.velocityNED;
  specificMechanicalEnergy =
    0.5 * vehicle.speed * vehicle.speed - 9.80665 * vehicle.positionNED[3];
  annotation (
    experiment(StartTime=0, StopTime=20, Tolerance=1e-9, Interval=0.1),
    Documentation(info="<html>
<p>Launches a point mass northward and upward with no applied force. Position
and velocity follow constant-gravity closed forms, and specific mechanical
energy remains constant.</p>
</html>"));
end BallisticTrajectory;
