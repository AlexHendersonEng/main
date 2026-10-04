within ModelicaAerospace.Sensors;
block IdealAltimeter "Ideal altitude sensor"
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.RealOutput measuredAltitude(unit="m");
equation
  measuredAltitude = altitude;
end IdealAltimeter;
