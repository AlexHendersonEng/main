within ModelicaAerospace.Mathematics.Blocks;
block Euler321ToQuaternion "Convert roll, pitch, yaw to a scalar-first quaternion"
  ModelicaAerospace.Interfaces.RealInput roll(unit="rad");
  ModelicaAerospace.Interfaces.RealInput pitch(unit="rad");
  ModelicaAerospace.Interfaces.RealInput yaw(unit="rad");
  ModelicaAerospace.Interfaces.QuaternionOutput quaternion;
equation
  quaternion = ModelicaAerospace.Mathematics.euler321ToQuaternion(roll, pitch, yaw);
end Euler321ToQuaternion;
