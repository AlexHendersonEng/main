within ModelicaAutomotive.Examples;
model DoubleTrackCornering "Four-corner braking turn with dynamic load transfer"
  parameter ModelicaAutomotive.Types.Mass mass = 1500;
  ModelicaAutomotive.VehicleDynamics.Planar.DoubleTrack vehicle(
    mass=mass,
    yawInertia=2500,
    geometry(
      wheelbase=2.8,
      frontTrack=1.6,
      rearTrack=1.6,
      centerOfMassToFrontAxle=1.2,
      centerOfMassHeight=0.55),
    initialState(longitudinalVelocity=20));
  output Real position[2](each unit="m");
  output Real longitudinalVelocity(unit="m/s");
  output Real lateralVelocity(unit="m/s");
  output Real yaw(unit="rad");
  output Real yawRate(unit="rad/s");
  output Real lateralAcceleration(unit="m/s2");
  output Real normalLoad[4](each unit="N");
  output Real yawMomentCommand(unit="N.m");
  output Real maneuverScale;
protected
  Real lateralForceScale;
  Real brakingForceScale;
equation
  lateralForceScale =
    if time < 1 then time
    elseif time < 2 then 1
    elseif time < 3 then 3 - time
    else 0;
  brakingForceScale = if time < 3 then 1 else 0;
  vehicle.tireLongitudinalForce =
    {-250, -250, -200, -200} * brakingForceScale;
  vehicle.tireLateralForce =
    {1200, 1200, 900, 900} * lateralForceScale;
  vehicle.steeringAngle = {0, 0, 0, 0};
  vehicle.externalLongitudinalForce = 0;
  vehicle.externalLateralForce = 0;
  vehicle.externalYawMoment = 100 * lateralForceScale;
  position = {vehicle.positionX, vehicle.positionY};
  longitudinalVelocity = vehicle.longitudinalVelocity;
  lateralVelocity = vehicle.lateralVelocity;
  yaw = vehicle.yaw;
  yawRate = vehicle.yawRate;
  lateralAcceleration = vehicle.lateralAcceleration;
  normalLoad = vehicle.normalLoad;
  yawMomentCommand = vehicle.externalYawMoment;
  maneuverScale = lateralForceScale;
  annotation (
    experiment(StartTime=0, StopTime=8, Tolerance=1e-8, Interval=0.01),
    Documentation(info="<html>
<p>Applies prescribed four-corner tire forces and a bounded yaw-moment input
during a braking turn to exercise double-track translation, yaw response,
and longitudinal/lateral load transfer. The example is a plant-composition
demonstration, not a tire-controller calibration case.</p>
</html>"));
end DoubleTrackCornering;
