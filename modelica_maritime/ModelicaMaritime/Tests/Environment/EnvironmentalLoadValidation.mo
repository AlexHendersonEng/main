within ModelicaMaritime.Tests.Environment;
model EnvironmentalLoadValidation "Validate relative drag and first-order wave loads"
  ModelicaMaritime.Environment.Loads.QuadraticMediumLoad drag(
    density=1.225,
    dragCoefficient={0.8, 1.1, 1.3},
    projectedArea={12, 20, 8},
    applicationPointBody={2, 0, -3});
  ModelicaMaritime.Environment.Loads.LinearWaveLoad wave(
    elevationGain={100, 200, 0, 0, 0, 50},
    velocityGain=[
      10, 0, 0;
      0, 20, 0;
      0, 0, 30;
      0, 0, 0;
      0, 0, 0;
      5, -5, 0]);
  output Real dragLoad[6];
  output Real waveLoad[6];
equation
  drag.relativeVelocityBody = {5, -2, 1};
  wave.waveElevation = 1.5;
  wave.waveVelocityBody = {0.4, -0.2, 0.1};
  dragLoad = drag.generalizedLoadBody;
  waveLoad = wave.generalizedLoadBody;
end EnvironmentalLoadValidation;
