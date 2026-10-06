within ModelicaAutomotive.Tests.Planar;
model KinematicCircle "Constant-speed constant-steer kinematic circle"
  ModelicaAutomotive.VehicleDynamics.Planar.KinematicBicycle bicycle;
  output Real positionX;
  output Real positionY;
  output Real yaw;
  output Real yawRate;
equation
  bicycle.speed = 10;
  bicycle.steeringAngle = 0.2;
  positionX = bicycle.positionX;
  positionY = bicycle.positionY;
  yaw = bicycle.yaw;
  yawRate = bicycle.yawRate;
end KinematicCircle;
