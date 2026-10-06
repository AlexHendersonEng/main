within ModelicaMaritime.Tests.VesselDynamics;
model FossenBuoyancy "Positive, neutral, and negative buoyancy limiting cases"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF positive(
    massProperties(
      mass=10,
      inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      displacedVolume=0.012),
    hydrodynamics(
      addedMass=zeros(6, 6),
      linearDamping=zeros(6, 6),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    waterDensity=1000);
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF neutral(
    massProperties(
      mass=10,
      inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      displacedVolume=0.01),
    hydrodynamics(
      addedMass=zeros(6, 6),
      linearDamping=zeros(6, 6),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    waterDensity=1000);
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF negative(
    massProperties(
      mass=10,
      inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      displacedVolume=0.008),
    hydrodynamics(
      addedMass=zeros(6, 6),
      linearDamping=zeros(6, 6),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    waterDensity=1000);
  output Real positionDown[3];
  output Real accelerationDown[3];
equation
  positive.generalizedForceBody = zeros(6);
  positive.currentVelocityNED = {0, 0, 0};
  neutral.generalizedForceBody = zeros(6);
  neutral.currentVelocityNED = {0, 0, 0};
  negative.generalizedForceBody = zeros(6);
  negative.currentVelocityNED = {0, 0, 0};
  positionDown = {
    positive.positionNED[3],
    neutral.positionNED[3],
    negative.positionNED[3]};
  accelerationDown = {
    positive.accelerationBody[3],
    neutral.accelerationBody[3],
    negative.accelerationBody[3]};
end FossenBuoyancy;
