within ModelicaAutomotive.Examples;
model RoughRoadRide "Deterministic four-track rough-road suspension response"
  ModelicaAutomotive.Suspension.FourCornerSuspension suspension(
    mass=1500,
    cornerStiffness=32000,
    cornerDamping=3800,
    frontAntiRollStiffness=7000,
    rearAntiRollStiffness=5000);
  output Real roadHeight[4](each unit="m");
  output Real heave(unit="m");
  output Real roll(unit="rad");
  output Real pitch(unit="rad");
  output Real suspensionForce[4](each unit="N");
equation
  roadHeight = {
    0.01 * sin(5 * time) + 0.004 * sin(13 * time),
    0.01 * sin(5 * time + 0.7) + 0.004 * sin(13 * time + 0.2),
    0.01 * sin(5 * time + 1.4) + 0.004 * sin(13 * time + 0.4),
    0.01 * sin(5 * time + 2.1) + 0.004 * sin(13 * time + 0.6)};
  suspension.roadHeight = roadHeight;
  suspension.roadVelocity = {
    0.05 * cos(5 * time) + 0.052 * cos(13 * time),
    0.05 * cos(5 * time + 0.7) + 0.052 * cos(13 * time + 0.2),
    0.05 * cos(5 * time + 1.4) + 0.052 * cos(13 * time + 0.4),
    0.05 * cos(5 * time + 2.1) + 0.052 * cos(13 * time + 0.6)};
  heave = suspension.heave;
  roll = suspension.roll;
  pitch = suspension.pitch;
  suspensionForce = suspension.suspensionForce;
  annotation (
    experiment(StartTime=0, StopTime=8, Tolerance=1e-8, Interval=0.005),
    Documentation(info="<html><p>Phase-shifted deterministic road inputs excite bounded heave, roll, and pitch ride responses.</p></html>"));
end RoughRoadRide;
