within ModelicaMaritime.Examples;
model MMGTurningCircle "Constant-rudder MMG-style turning maneuver"
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
      area=2,
      longitudinalPosition=-4.5,
      hullForcePosition=-1,
      liftGradient=6,
      axialInflowFactor=1,
      lateralInflowFactor=0.7,
      hullForceIncrease=0.1,
      steeringResistanceDeduction=0.1));
  output Real poseNED[3];
  output Real velocityBody[3];
  output Real hullLoadBody[3];
  output Real propellerLoadBody[3];
  output Real rudderLoadBody[3];
equation
  vessel.generalizedForceBody = {0, 0, 0};
  vessel.currentVelocityNED = {0, 0, 0};
  vessel.propellerRate = 5;
  vessel.rudderAngle = 0.17453292519943295;
  poseNED = vessel.poseNED;
  velocityBody = vessel.velocityBody;
  hullLoadBody = vessel.hullLoadBody;
  propellerLoadBody = vessel.propellerLoadBody;
  rudderLoadBody = vessel.rudderLoadBody;
  annotation (
    experiment(StartTime=0, StopTime=240, Tolerance=1e-8, Interval=0.2),
    Documentation(info="<html>
<p>Applies a constant 10-degree rudder command to a parameterized MMG-style
surface vessel. The coefficients are synthetic validation data rather than a
specific ship's proprietary or certified maneuvering data.</p>
</html>"));
end MMGTurningCircle;
