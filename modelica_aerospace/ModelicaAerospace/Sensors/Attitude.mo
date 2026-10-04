within ModelicaAerospace.Sensors;
block Attitude "Biased deterministic quaternion sensor with normalization"
  parameter Real bias[4] = {0, 0, 0, 0};
  parameter Real noiseAmplitude[4] = {0, 0, 0, 0};
  parameter Integer seed = 1;
  ModelicaAerospace.Interfaces.QuaternionInput quaternion;
  ModelicaAerospace.Interfaces.QuaternionOutput measuredQuaternion;
protected
  Real rawQuaternion[4];
  Real quaternionNorm;
equation
  for index in 1:4 loop
    rawQuaternion[index] = quaternion[index] + bias[index]
      + noiseAmplitude[index]
      * ModelicaAerospace.Environment.Wind.deterministicNoise(time, seed, index);
  end for;
  quaternionNorm = sqrt(
    rawQuaternion[1] * rawQuaternion[1]
    + rawQuaternion[2] * rawQuaternion[2]
    + rawQuaternion[3] * rawQuaternion[3]
    + rawQuaternion[4] * rawQuaternion[4]);
  assert(quaternionNorm > 1e-12, "Measured quaternion must remain nonzero");
  measuredQuaternion = rawQuaternion / quaternionNorm;
end Attitude;
