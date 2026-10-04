within ModelicaAerospace.Sensors;
block IdealAttitude "Ideal scalar-first quaternion sensor"
  ModelicaAerospace.Interfaces.QuaternionInput quaternion;
  ModelicaAerospace.Interfaces.QuaternionOutput measuredQuaternion;
equation
  measuredQuaternion = quaternion;
end IdealAttitude;
