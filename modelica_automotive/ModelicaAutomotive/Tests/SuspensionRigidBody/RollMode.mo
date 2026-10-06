within ModelicaAutomotive.Tests.SuspensionRigidBody;
model RollMode "Undamped symmetric sprung-body roll mode"
  ModelicaAutomotive.Suspension.FourCornerSuspension suspension(
    mass=1200,
    rollInertia=600,
    geometry(centerOfMassToFrontAxle=1.35),
    cornerStiffness=30000,
    cornerDamping=0,
    initialRoll=0.01);
  output Real roll;
equation
  suspension.roadHeight = {0, 0, 0, 0};
  suspension.roadVelocity = {0, 0, 0, 0};
  roll = suspension.roll;
end RollMode;
