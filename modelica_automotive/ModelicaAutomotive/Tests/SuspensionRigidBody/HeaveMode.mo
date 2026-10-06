within ModelicaAutomotive.Tests.SuspensionRigidBody;
model HeaveMode "Undamped symmetric sprung-body heave mode"
  ModelicaAutomotive.Suspension.FourCornerSuspension suspension(
    mass=1200,
    geometry(centerOfMassToFrontAxle=1.35),
    cornerStiffness=30000,
    cornerDamping=0,
    initialHeave=0.01);
  output Real heave;
equation
  suspension.roadHeight = {0, 0, 0, 0};
  suspension.roadVelocity = {0, 0, 0, 0};
  heave = suspension.heave;
end HeaveMode;
