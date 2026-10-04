within ModelicaAerospace.Sensors;
block IdealAirData "Ideal air-data sensor"
  ModelicaAerospace.Interfaces.RealInput trueAirspeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealInput angleOfAttack(unit="rad");
  ModelicaAerospace.Interfaces.RealInput sideslip(unit="rad");
  ModelicaAerospace.Interfaces.RealInput altitude(unit="m");
  ModelicaAerospace.Interfaces.RealOutput measuredAirspeed(unit="m/s");
  ModelicaAerospace.Interfaces.RealOutput measuredAngleOfAttack(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput measuredSideslip(unit="rad");
  ModelicaAerospace.Interfaces.RealOutput measuredAltitude(unit="m");
equation
  measuredAirspeed = trueAirspeed;
  measuredAngleOfAttack = angleOfAttack;
  measuredSideslip = sideslip;
  measuredAltitude = altitude;
end IdealAirData;
