within ModelicaMaritime.Tests.VesselDynamics;
model FossenInvalidMass "Exercise explicit total-inertia validation"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF vehicle(
    massProperties(
      mass=10,
      inertiaBody=[2, 0, 0; 0, 3, 0; 0, 0, 4],
      displacedVolume=0),
    hydrodynamics(
      addedMass=[0, 20, 0, 0, 0, 0;
                 20, 0, 0, 0, 0, 0;
                 0, 0, 0, 0, 0, 0;
                 0, 0, 0, 0, 0, 0;
                 0, 0, 0, 0, 0, 0;
                 0, 0, 0, 0, 0, 0],
      linearDamping=zeros(6, 6),
      quadraticDamping=zeros(6, 6)),
    initialState(
      positionNED={0, 0, 0},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    gravity=0);
  output Real acceleration[6];
equation
  vehicle.generalizedForceBody = zeros(6);
  vehicle.currentVelocityNED = {0, 0, 0};
  acceleration = vehicle.accelerationBody;
end FossenInvalidMass;
