within ModelicaAutomotive.Tests.SuspensionRigidBody;
model ComponentValidation "Validate suspension element force conventions"
  ModelicaAutomotive.Suspension.CornerSpringDamper corner(
    stiffness=30000,
    damping=2000,
    preload=1000);
  ModelicaAutomotive.Suspension.BumpStop stop(
    compressionTravel=0.05,
    reboundTravel=0.04,
    compressionStiffness=100000,
    reboundStiffness=50000);
  ModelicaAutomotive.Suspension.AntiRollBar bar(stiffness=10000);
  ModelicaAutomotive.Suspension.VerticalTire contact(
    stiffness=200000,
    damping=1000);
  ModelicaAutomotive.Suspension.VerticalTire separated(
    stiffness=200000,
    damping=1000);
  output Real cornerForce;
  output Real stopForce;
  output Real barForce[2];
  output Real contactImpulse(start=0, fixed=true);
  output Real separatedImpulse(start=0, fixed=true);
equation
  corner.compression = 0.02;
  corner.compressionRate = 0.1;
  stop.compression = 0.07;
  bar.leftCompression = 0.03;
  bar.rightCompression = 0.01;
  contact.roadHeight = 0.01;
  contact.wheelHeight = 0;
  contact.roadVelocity = 0;
  contact.wheelVelocity = 0;
  separated.roadHeight = -0.01;
  separated.wheelHeight = 0;
  separated.roadVelocity = 0;
  separated.wheelVelocity = 0;
  cornerForce = corner.forceOnBody;
  stopForce = stop.forceOnBody;
  barForce = {bar.leftForce, bar.rightForce};
  der(contactImpulse) = contact.normalForce;
  der(separatedImpulse) = separated.normalForce;
end ComponentValidation;
