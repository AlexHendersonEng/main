within ModelicaAutomotive.Tests.DriversSensorsControl;
model DeterministicSensorValidation "Biased, lagged, deterministic sensor response"
  ModelicaAutomotive.Sensors.DeterministicSensor sensor(
    gain=2,
    bias=1,
    timeConstant=0.5,
    noiseAmplitude=0,
    initialMeasurement=1);
  output Real trueSignal;
  output Real measurement;
  output Real idealValue;
  output Real measurementError;
equation
  trueSignal = 3;
  sensor.trueSignal = trueSignal;
  measurement = sensor.measurement;
  idealValue = sensor.idealValue;
  measurementError = sensor.measurementError;
end DeterministicSensorValidation;
