within ModelicaAerospace.Mathematics.Blocks;
block RotateVectorByQuaternion "Apply an active quaternion rotation"
  ModelicaAerospace.Interfaces.QuaternionInput quaternion;
  ModelicaAerospace.Interfaces.Vector3Input vector;
  ModelicaAerospace.Interfaces.Vector3Output rotated;
equation
  rotated = ModelicaAerospace.Mathematics.rotateVector(quaternion, vector);
end RotateVectorByQuaternion;
