within ModelicaAutomotive.Tests.Planar;
model DoubleTrackBalancedCorner "Lateral forces selected for zero yaw moment"
  parameter Real frontForce=1000;
  parameter Real rearForce=800;
  ModelicaAutomotive.VehicleDynamics.Planar.DoubleTrack vehicle(
    mass=1500,
    initialState(longitudinalVelocity=20));
  output Real lateralAcceleration;
  output Real yawRate;
  output Real normalLoad[4];
  output Real loadSum;
equation
  vehicle.tireLongitudinalForce = {0, 0, 0, 0};
  vehicle.tireLateralForce = {
    frontForce,
    frontForce,
    rearForce,
    rearForce};
  vehicle.steeringAngle = {0, 0, 0, 0};
  vehicle.externalLongitudinalForce = 0;
  vehicle.externalLateralForce = 0;
  vehicle.externalYawMoment = 0;
  lateralAcceleration = vehicle.lateralAcceleration;
  yawRate = vehicle.yawRate;
  normalLoad = vehicle.normalLoad;
  loadSum = sum(vehicle.normalLoad);
end DoubleTrackBalancedCorner;
