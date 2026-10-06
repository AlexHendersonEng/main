within ModelicaMaritime.Tests.VesselDynamics;
model MMGInvalidGeometry "Exercise explicit MMG propeller geometry validation"
  ModelicaMaritime.VesselDynamics.MMG.PropellerLoads propeller(
    density=1025,
    properties(
      diameter=0,
      wakeFraction=0.2,
      thrustDeduction=0.1,
      thrustCoefficient={0.2, 0, 0}));
  output Real load[3];
equation
  propeller.relativeVelocityBody = {5, 0, 0};
  propeller.rotationRate = 2;
  load = propeller.generalizedLoadBody;
end MMGInvalidGeometry;
