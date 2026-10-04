within ModelicaAerospace.Environment.Wind.Blocks;
block LinearWindShear "Linear wind shear in NED coordinates"
  parameter ModelicaAerospace.Types.Velocity referenceWindNED[3] = {0, 0, 0};
  parameter ModelicaAerospace.Types.Length referenceAltitude = 0;
  parameter Real gradientNED[3](each unit="1/s") = {0, 0, 0};
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.Vector3Output velocityNED(each unit="m/s");
equation
  velocityNED = ModelicaAerospace.Environment.Wind.linearWindShear(
    referenceWindNED,
    altitude,
    referenceAltitude,
    gradientNED);
end LinearWindShear;
