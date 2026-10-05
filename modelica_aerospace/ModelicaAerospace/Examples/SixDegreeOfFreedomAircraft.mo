within ModelicaAerospace.Examples;
model SixDegreeOfFreedomAircraft "Aerodynamically damped flat-Earth 6-DoF aircraft"
  parameter ModelicaAerospace.Types.Mass mass = 1000;
  parameter ModelicaAerospace.Types.Length referenceArea = 16;
  parameter ModelicaAerospace.Types.Length referenceSpan = 10;
  parameter ModelicaAerospace.Types.Length referenceChord = 1.6;
  parameter ModelicaAerospace.Types.Velocity trimSpeed = 80;
  parameter ModelicaAerospace.Types.Length altitude = 1000;
  final parameter Real density =
    ModelicaAerospace.Environment.Atmosphere.standardAtmosphereProperty(
      altitude,
      3);
  final parameter Real trimDynamicPressure =
    ModelicaAerospace.Environment.Atmosphere.dynamicPressure(density, trimSpeed);
  final parameter Real dragCoefficient = 0.03;
  final parameter Real liftCoefficient =
    mass * 9.80665 / (trimDynamicPressure * referenceArea);
  final parameter Real trimThrust =
    trimDynamicPressure * referenceArea * dragCoefficient;
  ModelicaAerospace.FlightDynamics.RigidBody.FlatEarth vehicle(
    massProperties(
      mass=mass,
      inertiaBody=[1800, 0, 0; 0, 2500, 0; 0, 0, 3200]),
    initialState(
      positionNED={0, 0, -altitude},
      velocityBody={trimSpeed, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}));
  ModelicaAerospace.Aerodynamics.StabilityDerivatives derivatives(
    forceBase={-dragCoefficient, 0, -liftCoefficient},
    forceBeta={0, -0.7, 0},
    momentBeta={-0.08, 0, 0.12},
    momentRate=[-0.5, 0, 0; 0, -1.0, 0; 0, 0, -0.3],
    momentControl=[0.12, 0, 0; 0, -0.7, 0; 0.03, 0, -0.15],
    referenceSpan=referenceSpan,
    referenceChord=referenceChord);
  ModelicaAerospace.Aerodynamics.CoefficientForcesMoments aerodynamics(
    referenceArea=referenceArea,
    referenceSpan=referenceSpan,
    referenceChord=referenceChord);
  output Real positionNED[3](each unit="m");
  output Real euler321[3](each unit="rad");
  output Real angularVelocityBody[3](each unit="rad/s");
  output Real airspeed(unit="m/s");
  output Real quaternionNorm;
equation
  derivatives.angleOfAttack = vehicle.angleOfAttack;
  derivatives.sideslip = vehicle.sideslip;
  derivatives.airspeed = vehicle.airspeed;
  derivatives.angularVelocityBody = vehicle.angularVelocityBody;
  derivatives.controlDeflection = {
    if time >= 1 and time < 1.5 then 0.05 else 0,
    0,
    0};
  aerodynamics.dynamicPressure =
    ModelicaAerospace.Environment.Atmosphere.dynamicPressure(
      density,
      vehicle.airspeed);
  aerodynamics.forceCoefficients = derivatives.forceCoefficients;
  aerodynamics.momentCoefficients = derivatives.momentCoefficients;
  vehicle.forceBody =
    aerodynamics.forceBody + {trimThrust, 0, 0};
  vehicle.momentBody = aerodynamics.momentBody;
  positionNED = vehicle.positionNED;
  euler321 = vehicle.euler321;
  angularVelocityBody = vehicle.angularVelocityBody;
  airspeed = vehicle.airspeed;
  quaternionNorm = vehicle.quaternionNorm;
  annotation (
    experiment(StartTime=0, StopTime=8, Tolerance=1e-8, Interval=0.02),
    Documentation(info="<html>
<p>Trims a flat-Earth rigid body at 80 m/s, applies a 0.5 s aileron pulse,
and lets aerodynamic rate and sideslip derivatives damp the response. The
example exercises all translational, rotational, and attitude states.</p>
</html>"));
end SixDegreeOfFreedomAircraft;
