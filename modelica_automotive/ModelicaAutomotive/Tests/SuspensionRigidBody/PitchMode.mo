within ModelicaAutomotive.Tests.SuspensionRigidBody;
model PitchMode "Undamped symmetric sprung-body pitch mode"
  ModelicaAutomotive.Suspension.FourCornerSuspension suspension(
    mass=1200,
    pitchInertia=1800,
    geometry(centerOfMassToFrontAxle=1.35),
    cornerStiffness=30000,
    cornerDamping=0,
    initialPitch=0.01);
  output Real pitch;
equation
  suspension.roadHeight = {0, 0, 0, 0};
  suspension.roadVelocity = {0, 0, 0, 0};
  pitch = suspension.pitch;
end PitchMode;
