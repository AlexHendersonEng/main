within ModelicaAerospace.Examples;
model LongitudinalAircraft "Atmosphere, aerodynamics, engine, and 3-DoF longitudinal flight"
  parameter ModelicaAerospace.Types.Mass mass = 1200;
  parameter ModelicaAerospace.Types.Length referenceArea = 16;
  parameter Real dragCoefficient = 0.035;
  parameter ModelicaAerospace.Types.Velocity trimSpeed = 70;
  parameter ModelicaAerospace.Types.Length altitude = 1000;
  final parameter Real trimDensity =
    ModelicaAerospace.Environment.Atmosphere.standardAtmosphereProperty(
      altitude,
      3);
  final parameter Real trimDynamicPressure =
    ModelicaAerospace.Environment.Atmosphere.dynamicPressure(trimDensity, trimSpeed);
  final parameter Real liftCoefficient =
    mass * 9.80665 / (trimDynamicPressure * referenceArea);
  final parameter Real trimThrust =
    trimDynamicPressure * referenceArea * dragCoefficient;
  final parameter Real trimThrottle = trimThrust / 5000;
  ModelicaAerospace.FlightDynamics.PointMass.FlightPath vehicle(
    mass=mass,
    initialPositionNED={0, 0, -altitude},
    initialSpeed=trimSpeed);
  ModelicaAerospace.Environment.Atmosphere.Blocks.StandardAtmosphere atmosphere;
  ModelicaAerospace.Environment.Atmosphere.Blocks.AirData airData;
  ModelicaAerospace.Aerodynamics.CoefficientForcesMoments aerodynamics(
    referenceArea=referenceArea);
  ModelicaAerospace.Propulsion.FirstOrderEngine engine(
    timeConstant=0.5,
    maximumThrust=5000,
    initialSpool=trimThrottle);
  output Real distanceNorth(unit="m");
  output Real altitudeOutput(unit="m");
  output Real speed(unit="m/s");
  output Real flightPathAngle(unit="rad");
  output Real mach;
  output Real thrust(unit="N");
equation
  atmosphere.altitude = -vehicle.positionNED[3];
  airData.trueAirspeed = vehicle.speed;
  airData.density = atmosphere.density;
  airData.speedOfSound = atmosphere.speedOfSound;
  aerodynamics.dynamicPressure = airData.dynamicPressure;
  aerodynamics.forceCoefficients = {
    -dragCoefficient,
    0,
    -liftCoefficient};
  aerodynamics.momentCoefficients = {0, 0, 0};
  engine.throttle = if time < 2 then trimThrottle else trimThrottle + 0.15;
  engine.enabled = true;
  vehicle.forceVelocityAxes = {
    engine.thrust + aerodynamics.forceBody[1],
    -aerodynamics.forceBody[3],
    0};
  distanceNorth = vehicle.positionNED[1];
  altitudeOutput = -vehicle.positionNED[3];
  speed = vehicle.speed;
  flightPathAngle = vehicle.flightPathAngle;
  mach = airData.mach;
  thrust = engine.thrust;
  annotation (
    experiment(StartTime=0, StopTime=15, Tolerance=1e-8, Interval=0.05),
    Documentation(info="<html>
<p>Starts a 1200 kg aircraft in level trim at 1 km and 70 m/s, then applies a
throttle step at 2 s. The composition couples atmosphere, air data,
coefficient scaling, engine spool, and flight-path dynamics.</p>
</html>"));
end LongitudinalAircraft;
