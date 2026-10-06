within ModelicaAutomotive.Tests.WheelsBrakes;
model WheelConstantTorque "Wheel response to constant net torque"
  ModelicaAutomotive.Wheels.RotationalDynamics wheel(
    inertia=2,
    rollingRadius=0.3,
    initialAngularVelocity=5);
  output Real angularVelocity;
  output Real angularAcceleration;
equation
  wheel.hubTorque = 20;
  wheel.brakeTorque = -4;
  wheel.longitudinalForce = 20;
  angularVelocity = wheel.angularVelocity;
  angularAcceleration = wheel.angularAcceleration;
end WheelConstantTorque;
