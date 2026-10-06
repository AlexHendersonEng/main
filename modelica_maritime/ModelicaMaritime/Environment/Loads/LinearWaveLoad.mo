within ModelicaMaritime.Environment.Loads;
block LinearWaveLoad "First-order load from wave elevation and particle velocity"
  parameter Real elevationGain[6]
    "Generalized load per metre of wave elevation";
  parameter Real velocityGain[6, 3]
    "Generalized load per wave-particle velocity component";
  ModelicaMaritime.Interfaces.RealInput waveElevation(unit="m");
  ModelicaMaritime.Interfaces.Vector3Input waveVelocityBody(each unit="m/s");
  ModelicaMaritime.Interfaces.Vector6Output generalizedLoadBody;
equation
  generalizedLoadBody =
    elevationGain * waveElevation + velocityGain * waveVelocityBody;
end LinearWaveLoad;
