within ModelicaAerospace.Sensors;
block IdealGPS "Ideal ECEF position and velocity sensor"
  ModelicaAerospace.Interfaces.Vector3Input positionECEF(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Input velocityECEF(each unit="m/s");
  ModelicaAerospace.Interfaces.Vector3Output measuredPositionECEF(each unit="m");
  ModelicaAerospace.Interfaces.Vector3Output measuredVelocityECEF(each unit="m/s");
equation
  measuredPositionECEF = positionECEF;
  measuredVelocityECEF = velocityECEF;
end IdealGPS;
