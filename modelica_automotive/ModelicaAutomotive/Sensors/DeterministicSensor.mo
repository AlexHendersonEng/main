within ModelicaAutomotive.Sensors;
block DeterministicSensor "Configurable scalar sensor with deterministic error and lag"
  parameter Real gain = 1;
  parameter Real bias = 0;
  parameter Real timeConstant(unit="s") = 0.01;
  parameter Real noiseAmplitude = 0;
  parameter Real noiseFrequency(unit="Hz") = 1;
  parameter Real minimumOutput = -1e100;
  parameter Real maximumOutput = 1e100;
  parameter Real initialMeasurement = 0;
  ModelicaAutomotive.Interfaces.RealInput trueSignal;
  ModelicaAutomotive.Interfaces.RealOutput measurement;
  ModelicaAutomotive.Interfaces.RealOutput idealValue;
  ModelicaAutomotive.Interfaces.RealOutput measurementError;
protected
  Real measurementState(start=initialMeasurement, fixed=true);
  Real disturbedValue;
equation
  assert(timeConstant > 0, "timeConstant must be positive");
  assert(noiseFrequency >= 0, "noiseFrequency must not be negative");
  assert(maximumOutput > minimumOutput,
    "maximumOutput must exceed minimumOutput");
  idealValue = gain * trueSignal + bias;
  disturbedValue = min(max(
    idealValue + noiseAmplitude * sin(6.283185307179586
      * noiseFrequency * time),
    minimumOutput),
    maximumOutput);
  der(measurementState) = (disturbedValue - measurementState) / timeConstant;
  measurement = measurementState;
  measurementError = measurement - trueSignal;
end DeterministicSensor;
