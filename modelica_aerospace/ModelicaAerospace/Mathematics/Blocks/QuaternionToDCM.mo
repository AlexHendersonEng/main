within ModelicaAerospace.Mathematics.Blocks;
block QuaternionToDCM "Convert a quaternion to an active rotation matrix"
  ModelicaAerospace.Interfaces.QuaternionInput quaternion;
  ModelicaAerospace.Interfaces.Matrix3Output dcm;
equation
  dcm = ModelicaAerospace.Mathematics.quaternionToDCM(quaternion);
end QuaternionToDCM;
