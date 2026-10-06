within ModelicaAutomotive.Examples;
model BicycleComparison "Compare low-speed kinematic and dynamic bicycle responses"
  ModelicaAutomotive.VehicleDynamics.Planar.KinematicBicycle kinematic;
  ModelicaAutomotive.VehicleDynamics.Planar.DynamicBicycle dynamic;
  output Real steeringAngle(unit="rad");
  output Real kinematicYaw(unit="rad");
  output Real dynamicYaw(unit="rad");
  output Real kinematicPosition[2];
  output Real dynamicPosition[2];
equation
  steeringAngle = 0.08;
  kinematic.speed = 3;
  kinematic.steeringAngle = steeringAngle;
  dynamic.longitudinalVelocity = 3;
  dynamic.steeringAngle = steeringAngle;
  kinematicYaw = kinematic.yaw;
  dynamicYaw = dynamic.yaw;
  kinematicPosition = {kinematic.positionX, kinematic.positionY};
  dynamicPosition = {dynamic.positionX, dynamic.positionY};
  annotation (
    experiment(StartTime=0, StopTime=8, Tolerance=1e-8, Interval=0.02),
    Documentation(info="<html>
<p>Compares reduced-order kinematic and linear-tire dynamic bicycle models at
low speed, where their yaw and path responses should approach one another.</p>
</html>"));
end BicycleComparison;
