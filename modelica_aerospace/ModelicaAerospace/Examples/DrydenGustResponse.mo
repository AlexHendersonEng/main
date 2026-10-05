within ModelicaAerospace.Examples;
model DrydenGustResponse "Deterministic vertical response to low-altitude Dryden turbulence"
  parameter ModelicaAerospace.Types.Velocity trueAirspeed = 60;
  ModelicaAerospace.Environment.Wind.Blocks.DrydenTurbulence turbulence(
    seed=23,
    windSpeedAt20Feet=12);
  output Real turbulenceBody[3](each unit="m/s");
  output Real verticalVelocity(unit="m/s", start=0, fixed=true);
  output Real verticalAcceleration(unit="m/s2");
equation
  turbulence.altitude = 100;
  turbulence.trueAirspeed = trueAirspeed;
  turbulenceBody = turbulence.turbulenceBody;
  verticalAcceleration = 0.8 * turbulenceBody[3] - 0.6 * verticalVelocity;
  der(verticalVelocity) = verticalAcceleration;
  annotation (
    experiment(StartTime=0, StopTime=40, Tolerance=1e-8, Interval=0.05),
    Documentation(info="<html>
<p>Drives a damped vertical-velocity response with seeded MIL-F-8785C
low-altitude Dryden turbulence. The fixed seed makes regression traces
repeatable while the response remains solver-continuous.</p>
</html>"));
end DrydenGustResponse;
