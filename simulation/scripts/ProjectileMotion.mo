model ProjectileMotion
  // Model description
  "Proportional-navigation interceptor pursuing a ballistic target"

  // Model parameters
  parameter ModelicaAerospace.Types.Mass targetMass = 50;
  parameter ModelicaAerospace.Types.Mass interceptorMass = 20;
  parameter Real navigationGain = 4 "True proportional navigation gain N'";
  parameter ModelicaAerospace.Types.Length captureRadius = 5
    "Range below which the target is considered intercepted";
  parameter ModelicaAerospace.Types.Acceleration maxAcceleration = 300
    "Interceptor actuator limit; also bounds the LOS-rate singularity as range to 0";
  parameter ModelicaAerospace.Types.Acceleration gravityNED[3] = {0, 0, 9.80665};
  parameter ModelicaAerospace.Types.PointMassInitialState targetInitialState(
    positionNED={0, 0, 0},
    velocityNED={60, 0, -90});
  parameter ModelicaAerospace.Types.PointMassInitialState interceptorInitialState(
    positionNED={900, 300, -2},
    velocityNED={-120, -40, -20});

  // Create point mass models for the target and interceptor
  ModelicaAerospace.FlightDynamics.PointMass.Cartesian target(
    mass=targetMass,
    gravityNED=gravityNED,
    initialState=targetInitialState);
  ModelicaAerospace.FlightDynamics.PointMass.Cartesian interceptor(
    mass=interceptorMass,
    gravityNED=gravityNED,
    initialState=interceptorInitialState);

  // Output variables
  Real targetPositionNED[3](each unit="m");
  Real interceptorPositionNED[3](each unit="m");
  Real range(unit="m");
  Real closingVelocity(unit="m/s");
  Real commandedAcceleration[3](each unit="m/s2");
  Boolean intercepted;
protected
  // Intermediate variables
  Real relativePosition[3];
  Real relativeVelocity[3];
  Real rangeSquared;
  Real losRate[3] "Line-of-sight angular velocity vector";
  Real rawCommand[3] "Unsaturated proportional navigation command";
  Real rawCommandMagnitude(unit="m/s2");
equation
  // The target falls freely; only the interceptor is powered and guided.
  target.forceNED = {0, 0, 0};

  // Compute the relative position and velocity of the target with respect to the interceptor.
  relativePosition = target.positionNED - interceptor.positionNED;
  relativeVelocity = target.velocityNED - interceptor.velocityNED;
  rangeSquared = ModelicaAerospace.Mathematics.dot3(relativePosition, relativePosition);
  range = sqrt(rangeSquared);
  closingVelocity =
    -ModelicaAerospace.Mathematics.dot3(relativePosition, relativeVelocity) / max(range, 1e-6);
  losRate =
    ModelicaAerospace.Mathematics.cross3(relativePosition, relativeVelocity) / max(rangeSquared, 1e-6);

  // True proportional navigation: commanded acceleration is normal to the
  // interceptor's velocity, proportional to closing speed and LOS rotation.
  rawCommand =
    navigationGain * ModelicaAerospace.Mathematics.cross3(losRate, interceptor.velocityNED);
  rawCommandMagnitude = ModelicaAerospace.Mathematics.norm3(rawCommand);

  // Saturating to a realistic actuator limit also removes the unbounded spike that
  // the LOS-rate singularity would otherwise produce as range approaches zero, which
  // keeps fixed-step (co-simulation) integration stable near intercept.
  commandedAcceleration =
    rawCommand * (min(rawCommandMagnitude, maxAcceleration) / max(rawCommandMagnitude, 1e-6));

  // Thrust vector control cancels gravity and supplies the guidance command.
  interceptor.forceNED = interceptorMass * (commandedAcceleration - gravityNED);

  // Interception criterion
  targetPositionNED = target.positionNED;
  interceptorPositionNED = interceptor.positionNED;
  intercepted = range <= captureRadius;

  // Experimental setup
  annotation (
    experiment(StartTime=0, StopTime=10, Tolerance=1e-9, Interval=0.01),
    Documentation(info="<html>
<p>A ballistic target is launched with no applied force. A powered
interceptor uses true proportional navigation, steering on the
line-of-sight rate between itself and the target, to fly an intercept
course. The interceptor cancels its own weight with thrust vector control
so the full commanded acceleration is available for guidance.</p>
</html>"));
end ProjectileMotion;
