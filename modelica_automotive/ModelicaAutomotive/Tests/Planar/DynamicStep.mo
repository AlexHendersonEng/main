within ModelicaAutomotive.Tests.Planar;
model DynamicStep "Constant-speed dynamic bicycle step steer"
  ModelicaAutomotive.VehicleDynamics.Planar.DynamicBicycle bicycle;
  output Real lateralVelocity;
  output Real yawRate;
  output Real lateralAcceleration;
  output Real frontSlipAngle;
  output Real rearSlipAngle;
equation
  bicycle.longitudinalVelocity = 15;
  bicycle.steeringAngle = 0.05;
  lateralVelocity = bicycle.lateralVelocity;
  yawRate = bicycle.yawRate;
  lateralAcceleration = bicycle.lateralAcceleration;
  frontSlipAngle = bicycle.frontSlipAngle;
  rearSlipAngle = bicycle.rearSlipAngle;
end DynamicStep;
