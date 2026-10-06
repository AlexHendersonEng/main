within ModelicaMaritime.Examples;
model SurfaceWaveResponse "Fossen-style six-DoF response to a regular wave"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF vessel(
    massProperties(
      mass=1000,
      centerOfGravityBody={0, 0, 0.3},
      centerOfBuoyancyBody={0, 0, -0.3},
      inertiaBody=[800, 0, 0; 0, 1200, 0; 0, 0, 1500],
      displacedVolume=1000 / 1025),
    hydrodynamics(
      addedMass=diagonal({200, 400, 500, 150, 250, 300}),
      linearDamping=diagonal({300, 500, 700, 500, 700, 800}),
      quadraticDamping=diagonal({80, 120, 160, 80, 100, 120})),
    initialState(
      positionNED={0, 0, 1},
      velocityBody={0, 0, 0},
      quaternionBodyToNED={1, 0, 0, 0},
      angularVelocityBody={0, 0, 0}),
    quaternionStabilization=50);
  ModelicaMaritime.Environment.Waves.RegularWave wave(
    amplitude=0.5,
    period=6,
    direction=0.4);
  ModelicaMaritime.Environment.Loads.LinearWaveLoad waveLoad(
    elevationGain={0, 0, 1500, 0, 3500, 0},
    velocityGain=[
      200, 0, 0;
      0, 300, 0;
      0, 0, 500;
      0, 0, 0;
      0, 0, 300;
      0, 200, 0]);
  output Real positionNED[3];
  output Real velocityBody[6];
  output Real quaternion[4];
  output Real waveElevation;
  output Real waveLoadBody[6];
  output Real quaternionNorm;
equation
  vessel.currentVelocityNED = {0.2, 0, 0};
  wave.positionNED = vessel.positionNED;
  waveLoad.waveElevation = wave.elevation;
  waveLoad.waveVelocityBody =
    transpose(vessel.rotationBodyToNED) * wave.waterVelocityNED;
  vessel.generalizedForceBody = waveLoad.generalizedLoadBody
    + {1000, 0, 0, 0, 0, 0};
  positionNED = vessel.positionNED;
  velocityBody = vessel.velocityBody;
  quaternion = vessel.quaternionBodyToNED;
  waveElevation = wave.elevation;
  waveLoadBody = waveLoad.generalizedLoadBody;
  quaternionNorm = vessel.quaternionNorm;
  annotation (
    experiment(StartTime=0, StopTime=60, Tolerance=1e-8, Interval=0.05),
    Documentation(info="<html>
<p>A neutrally buoyant Fossen-style vehicle advances in a deterministic
regular wave. First-order elevation and particle-velocity excitation drive
bounded heave, pitch, and yaw response. This is a system-integration example,
not a radiation/diffraction seakeeping solution.</p>
</html>"));
end SurfaceWaveResponse;
