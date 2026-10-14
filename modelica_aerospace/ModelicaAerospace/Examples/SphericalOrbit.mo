within ModelicaAerospace.Examples;
model SphericalOrbit "Circular low-Earth orbit with rotating-Earth rigid-body dynamics"
  parameter ModelicaAerospace.Types.Length orbitalAltitude = 400000;
  final parameter ModelicaAerospace.Types.Length orbitalRadius =
    ModelicaAerospace.Constants.WGS84.semiMajorAxis + orbitalAltitude;
  final parameter Real gravitationalParameter(unit="m3/s2") =
    ModelicaAerospace.Constants.WGS84.gravitationalParameter;
  final parameter ModelicaAerospace.Types.AngularVelocity earthRotationRate =
    ModelicaAerospace.Constants.WGS84.earthRotationRate;
  final parameter ModelicaAerospace.Types.Velocity inertialOrbitalSpeed =
    sqrt(gravitationalParameter / orbitalRadius);
  ModelicaAerospace.FlightDynamics.RigidBody.SphericalEarth vehicle(
    massProperties(
      mass=1000,
      inertiaBody=[1200, 0, 0; 0, 1500, 0; 0, 0, 1800]),
    initialState(
      positionECEF={orbitalRadius, 0, 0},
      velocityECEF={
        0,
        inertialOrbitalSpeed - earthRotationRate * orbitalRadius,
        0},
      quaternionBodyToECEF={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}));
  output Real positionECEF[3](each unit="m");
  output Real altitude(unit="m");
  output Real latitude(unit="rad");
  output Real longitude(unit="rad");
  output Real specificMechanicalEnergy(unit="m2/s2");
  output Real specificAngularMomentum[3](each unit="m2/s");
  output Real quaternionNorm;
equation
  vehicle.forceBody = {0, 0, 0};
  vehicle.momentBody = {0, 0, 0};
  positionECEF = vehicle.positionECEF;
  altitude = vehicle.altitude;
  latitude = vehicle.latitude;
  longitude = vehicle.longitude;
  specificMechanicalEnergy = vehicle.specificMechanicalEnergy;
  specificAngularMomentum = vehicle.specificAngularMomentum;
  quaternionNorm = vehicle.quaternionNorm;
  annotation (
    experiment(StartTime=0, StopTime=6000, Tolerance=1e-8, Interval=5),
    Documentation(info="<html>
<p>Propagates an initially circular 400 km orbit with the rotating-Earth
spherical rigid-body model. The initial ECEF velocity accounts for Earth
rotation, while central gravity, Coriolis acceleration, and centrifugal
acceleration produce a closed inertial orbit. Position, geodetic altitude,
specific energy, angular momentum, and attitude norm expose the trajectory
and conservation diagnostics.</p>
</html>"));
end SphericalOrbit;
