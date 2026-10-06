within ModelicaAutomotive.Tests.DriversSensorsControl;
model PathTracking "Look-ahead steering tracks a parallel target path"
  ModelicaAutomotive.Drivers.LookAheadSteering driver(
    wheelbase=2.7,
    lookAheadTime=0.6,
    minimumLookAhead=3,
    headingGain=1.5,
    maximumSteeringAngle=0.4);
  ModelicaAutomotive.VehicleDynamics.Planar.KinematicBicycle vehicle;
  output Real targetLateralPosition;
  output Real lateralPosition;
  output Real lateralError;
  output Real yaw;
  output Real steeringCommand;
equation
  targetLateralPosition = 2;
  driver.lateralError = targetLateralPosition - vehicle.positionY;
  driver.headingError = -vehicle.yaw;
  driver.speed = 10;
  vehicle.speed = 10;
  vehicle.steeringAngle = driver.steeringCommand;
  lateralPosition = vehicle.positionY;
  lateralError = driver.lateralError;
  yaw = vehicle.yaw;
  steeringCommand = driver.steeringCommand;
end PathTracking;
