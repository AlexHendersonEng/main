within ModelicaAerospace.Environment.Gravity.Blocks;
block NormalGravity "WGS-84 normal gravity in local NED coordinates"
  ModelicaAerospace.Interfaces.RealInput latitude(unit="rad");
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.Vector3Output gravityNED(each unit="m/s2");
equation
  gravityNED = {0, 0,
    ModelicaAerospace.Environment.Gravity.normalGravity(latitude, altitude)};
end NormalGravity;
