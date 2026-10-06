within ModelicaMaritime.Examples;
model UnderwaterFreeDecay "Stable underwater vehicle roll and pitch free decay"
  ModelicaMaritime.VesselDynamics.Fossen.SixDOF vehicle(
    massProperties(
      mass=500,
      centerOfGravityBody={0, 0, 0.15},
      centerOfBuoyancyBody={0, 0, -0.15},
      inertiaBody=[200, 0, 0; 0, 300, 0; 0, 0, 350],
      displacedVolume=500 / 1025),
    hydrodynamics(
      addedMass=diagonal({100, 200, 250, 50, 80, 100}),
      linearDamping=diagonal({100, 200, 250, 150, 180, 200}),
      quadraticDamping=diagonal({50, 100, 120, 30, 40, 50})),
    initialState(
      positionNED={0, 0, 50},
      velocityBody={1, 0, 0},
      quaternionBodyToNED={
        0.9924038765061041,
        0.08682408883346517,
        -0.08682408883346517,
        0.007596123493895969},
      angularVelocityBody={0, 0, 0}),
    waterDensity=1025);
  output Real positionNED[3];
  output Real velocityBody[6];
  output Real quaternion[4];
  output Real quaternionNorm;
  output Real dissipationPower;
equation
  vehicle.generalizedForceBody = zeros(6);
  vehicle.currentVelocityNED = {0.2, 0, 0};
  positionNED = vehicle.positionNED;
  velocityBody = vehicle.velocityBody;
  quaternion = vehicle.quaternionBodyToNED;
  quaternionNorm = vehicle.quaternionNorm;
  dissipationPower = vehicle.dissipationPower;
  annotation (
    experiment(StartTime=0, StopTime=40, Tolerance=1e-8, Interval=0.05),
    Documentation(info="<html>
<p>A neutrally buoyant underwater vehicle starts with roll and pitch offsets
and forward speed in a steady current. Hydrostatic restoring and hydrodynamic
damping return the attitude toward equilibrium while relative speed decays.</p>
</html>"));
end UnderwaterFreeDecay;
