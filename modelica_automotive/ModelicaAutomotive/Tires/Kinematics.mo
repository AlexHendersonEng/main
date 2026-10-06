within ModelicaAutomotive.Tires;
block Kinematics "Calculate regularized longitudinal slip ratio and slip angle"
  parameter ModelicaAutomotive.Types.Length rollingRadius = 0.3;
  parameter ModelicaAutomotive.Types.Velocity velocityRegularization = 0.1;
  ModelicaAutomotive.Interfaces.RealInput longitudinalVelocity(unit="m/s")
    "Wheel-center velocity in the wheel x direction";
  ModelicaAutomotive.Interfaces.RealInput lateralVelocity(unit="m/s")
    "Wheel-center velocity in the wheel y direction";
  ModelicaAutomotive.Interfaces.RealInput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput slipRatio;
  ModelicaAutomotive.Interfaces.RealOutput slipAngle(unit="rad");
protected
  Real referenceSpeed(unit="m/s");
equation
  assert(rollingRadius > 0, "rollingRadius must be positive");
  assert(velocityRegularization > 0, "velocityRegularization must be positive");
  referenceSpeed = sqrt(
    longitudinalVelocity * longitudinalVelocity
    + velocityRegularization * velocityRegularization);
  slipRatio = (rollingRadius * angularVelocity - longitudinalVelocity) / referenceSpeed;
  slipAngle = atan2(-lateralVelocity, referenceSpeed);
end Kinematics;
