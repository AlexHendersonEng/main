within ModelicaAutomotive.Tests.SuspensionRigidBody;
model BankEquilibrium "Damped suspension settles to a constant banked road"
  parameter Real bank=0.04;
  ModelicaAutomotive.Suspension.FourCornerSuspension suspension(
    mass=1200,
    geometry(centerOfMassToFrontAxle=1.35),
    cornerStiffness=30000,
    cornerDamping=5000);
  output Real heave;
  output Real roll;
  output Real pitch;
  output Real forceSum;
equation
  suspension.roadHeight = {
    0.8 * tan(bank),
    -0.8 * tan(bank),
    0.8 * tan(bank),
    -0.8 * tan(bank)};
  suspension.roadVelocity = {0, 0, 0, 0};
  heave = suspension.heave;
  roll = suspension.roll;
  pitch = suspension.pitch;
  forceSum = sum(suspension.suspensionForce);
end BankEquilibrium;
