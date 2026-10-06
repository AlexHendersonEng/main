within ModelicaMaritime.Environment.Wind.Blocks;
block GustingWind "Mean NED wind with deterministic sinusoidal gust"
  parameter ModelicaMaritime.Types.Velocity meanVelocityNED[3] = {0, 0, 0};
  parameter ModelicaMaritime.Types.Velocity gustAmplitudeNED[3] = {0, 0, 0};
  parameter Real gustFrequency(unit="Hz", min=0) = 0.1;
  parameter ModelicaMaritime.Types.Angle phase = 0;
  ModelicaMaritime.Interfaces.Vector3Output velocityNED(each unit="m/s");
equation
  velocityNED = ModelicaMaritime.Environment.Wind.gustVelocityNED(
    time,
    meanVelocityNED,
    gustAmplitudeNED,
    gustFrequency,
    phase);
end GustingWind;
