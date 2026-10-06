within ModelicaAutomotive.Examples;
model SuspensionBumpResponse "Left-front road bump through a damped four-corner suspension"
  ModelicaAutomotive.Suspension.FourCornerSuspension suspension(
    mass=1500,
    cornerStiffness=32000,
    cornerDamping=3500,
    frontAntiRollStiffness=8000,
    rearAntiRollStiffness=6000);
  output Real roadHeight[4];
  output Real heave(unit="m");
  output Real roll(unit="rad");
  output Real pitch(unit="rad");
  output Real suspensionForce[4];
equation
  roadHeight = {
    if time >= 1 and time <= 1.2 then
      0.05 * sin(3.141592653589793 * (time - 1) / 0.2) else 0,
    0,
    0,
    0};
  suspension.roadHeight = roadHeight;
  suspension.roadVelocity = {
    if time >= 1 and time <= 1.2 then
      0.05 * 3.141592653589793 / 0.2
        * cos(3.141592653589793 * (time - 1) / 0.2) else 0,
    0,
    0,
    0};
  heave = suspension.heave;
  roll = suspension.roll;
  pitch = suspension.pitch;
  suspensionForce = suspension.suspensionForce;
  annotation (
    experiment(StartTime=0, StopTime=5, Tolerance=1e-8, Interval=0.005),
    Documentation(info="<html>
<p>A smooth 50 mm half-sine bump at the left-front corner excites damped
heave, roll, and pitch motion.</p>
</html>"));
end SuspensionBumpResponse;
