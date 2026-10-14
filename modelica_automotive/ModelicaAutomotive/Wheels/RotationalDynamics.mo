within ModelicaAutomotive.Wheels;
block RotationalDynamics "Wheel rotational dynamics with hub, brake, and tire torques"
  parameter ModelicaAutomotive.Types.Inertia inertia = 1.2;
  parameter ModelicaAutomotive.Types.Length rollingRadius = 0.3;
  parameter ModelicaAutomotive.Types.AngularVelocity initialAngularVelocity = 0;
  ModelicaAutomotive.Interfaces.RealInput hubTorque(unit="N.m")
    "Positive drive torque";
  ModelicaAutomotive.Interfaces.RealInput brakeTorque(unit="N.m")
    "Signed brake torque, normally opposite angular velocity";
  ModelicaAutomotive.Interfaces.RealInput longitudinalForce(unit="N")
    "Positive forward tire force";
  ModelicaAutomotive.Interfaces.RealOutput angularVelocity(unit="rad/s");
  ModelicaAutomotive.Interfaces.RealOutput angularAcceleration(unit="rad/s2");
  ModelicaAutomotive.Interfaces.RealOutput tireReactionTorque(unit="N.m");
protected
  Real angularVelocityState(start=initialAngularVelocity, fixed=true);
equation
  assert(inertia > 0, "Wheel inertia must be positive");
  assert(rollingRadius > 0, "Wheel rollingRadius must be positive");
  tireReactionTorque = -longitudinalForce * rollingRadius;
  angularAcceleration =
    (hubTorque + brakeTorque + tireReactionTorque) / inertia;
  der(angularVelocityState) = angularAcceleration;
  angularVelocity = angularVelocityState;
end RotationalDynamics;
