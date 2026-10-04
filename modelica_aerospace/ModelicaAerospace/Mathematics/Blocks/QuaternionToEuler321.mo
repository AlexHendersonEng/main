within ModelicaAerospace.Mathematics.Blocks;
block QuaternionToEuler321 "Convert a quaternion to roll, pitch, yaw"
  ModelicaAerospace.Interfaces.QuaternionInput quaternion;
  ModelicaAerospace.Interfaces.Vector3Output euler(each unit="rad")
    "{roll, pitch, yaw}";
equation
  euler = ModelicaAerospace.Mathematics.quaternionToEuler321(quaternion);
end QuaternionToEuler321;
