within ModelicaAutomotive.Examples;
model PlanarManeuver "Step and swept-sine steering of a dynamic bicycle"
  ModelicaAutomotive.Steering.FirstOrderSteering steering(
    timeConstant=0.12,
    maximumAngle=0.2,
    maximumRate=0.8);
  ModelicaAutomotive.VehicleDynamics.Planar.DynamicBicycle vehicle;
  output Real steeringCommand(unit="rad");
  output Real steeringAngle(unit="rad");
  output Real position[2];
  output Real yaw(unit="rad");
  output Real yawRate(unit="rad/s");
  output Real lateralAcceleration(unit="m/s2");
equation
  steeringCommand =
    if time < 1 then 0
    elseif time < 4 then 0.05
    else 0.04 * sin(0.8 * (time - 4) * (time - 4));
  steering.command = steeringCommand;
  vehicle.longitudinalVelocity = 15;
  vehicle.steeringAngle = steering.angle;
  steeringAngle = steering.angle;
  position = {vehicle.positionX, vehicle.positionY};
  yaw = vehicle.yaw;
  yawRate = vehicle.yawRate;
  lateralAcceleration = vehicle.lateralAcceleration;
  annotation (
    experiment(StartTime=0, StopTime=10, Tolerance=1e-8, Interval=0.01),
    Documentation(info="<html>
<p>Applies a step steer followed by a swept-sine command through a
rate-limited steering actuator to the dynamic bicycle model.</p>
</html>"));
end PlanarManeuver;
