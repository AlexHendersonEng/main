within ModelicaAutomotive.Examples;
model FullBodyManeuver "Forward acceleration with a bounded roll-moment pulse"
  ModelicaAutomotive.VehicleDynamics.RigidBody.FullBody vehicle(
    massProperties(
      mass=1500,
      inertiaBody=[650, 0, 0; 0, 2200, 0; 0, 0, 2500]),
    initialState(velocityBody={15, 0, 0}));
  output Real positionWorld[3];
  output Real velocityBody[3];
  output Real angularVelocityBody[3];
  output Real quaternion[4];
  output Real quaternionNorm;
equation
  vehicle.forceBody = {1500, 0, 1500 * 9.80665};
  vehicle.momentBody = {
    if time >= 1 and time <= 1.5 then 400 else
      if time > 1.5 and time <= 2 then -400 else 0,
    0,
    0};
  positionWorld = vehicle.positionWorld;
  velocityBody = vehicle.velocityBody;
  angularVelocityBody = vehicle.angularVelocityBody;
  quaternion = vehicle.quaternionBodyToWorld;
  quaternionNorm = vehicle.quaternionNorm;
  annotation (
    experiment(StartTime=0, StopTime=5, Tolerance=1e-8, Interval=0.01),
    Documentation(info="<html>
<p>Applies forward force and a zero-net-impulse roll-moment pulse to the
quaternion full-body vehicle plant.</p>
</html>"));
end FullBodyManeuver;
