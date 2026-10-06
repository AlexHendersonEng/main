within ModelicaMaritime.Tests.VesselDynamics;
model MMGZigZag "Heading-triggered 10-degree/10-degree MMG zig-zag maneuver"
  parameter Real targetHeading(unit="rad") = 0.17453292519943295;
  parameter Real rudderMagnitude(unit="rad") = 0.17453292519943295;
  ModelicaMaritime.VesselDynamics.MMG.Planar3DOF vessel(
    massProperties(
      mass=10000,
      centerOfGravityX=0,
      yawInertia=200000),
    hydrodynamics(
      addedMass=[2000, 0, 0; 0, 5000, 0; 0, 0, 50000],
      linearDamping=zeros(3, 3),
      quadraticDamping=zeros(3, 3)),
    initialState(
      positionNED={0, 0},
      heading=0,
      velocityBody={5, 0},
      yawRate=0),
    density=1025,
    referenceLength=10,
    referenceDraft=2,
    hullCoefficients(
      surgeConstant=-0.02,
      surgeSway2=-0.2,
      surgeYaw2=-0.2,
      swayLinear=-1.5,
      swayCubic=-2,
      yawRate=-1,
      yawRateCubic=-1),
    propellerProperties(
      diameter=1,
      wakeFraction=0,
      thrustDeduction=0,
      thrustCoefficient={0.2, 0, 0}),
    rudderProperties(
      area=0.5,
      longitudinalPosition=-4.5,
      hullForcePosition=-1,
      liftGradient=6,
      axialInflowFactor=1,
      lateralInflowFactor=0.7,
      hullForceIncrease=0.1,
      steeringResistanceDeduction=0.1));
  discrete Integer phase(start=1, fixed=true);
  Real commandedRudder(unit="rad");
  output Real poseNED[3];
  output Real velocityBody[3];
  output Real rudderAngle(unit="rad");
  output Integer maneuverPhase;
equation
  when pre(phase) == 1 and vessel.poseNED[3] >= targetHeading then
    phase = 2;
  elsewhen pre(phase) == 2 and vessel.poseNED[3] <= -targetHeading then
    phase = 3;
  end when;
  commandedRudder = if phase == 2 then -rudderMagnitude else rudderMagnitude;
  vessel.generalizedForceBody = {0, 0, 0};
  vessel.currentVelocityNED = {0, 0, 0};
  vessel.propellerRate = 5;
  vessel.rudderAngle = commandedRudder;
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  rudderAngle = commandedRudder;
  maneuverPhase = phase;
end MMGZigZag;
