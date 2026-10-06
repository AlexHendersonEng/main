within ModelicaMaritime.Interfaces;
partial block PartialEnvironment "Common local ocean-environment interface"
  ModelicaMaritime.Interfaces.RealInput depth(unit="m")
    "Depth below the reference water surface, positive down";
  ModelicaMaritime.Interfaces.RealOutput temperature(unit="K");
  ModelicaMaritime.Interfaces.RealOutput pressure(unit="Pa");
  ModelicaMaritime.Interfaces.RealOutput density(unit="kg/m3");
  ModelicaMaritime.Interfaces.Vector3Output currentVelocityNED(each unit="m/s");
end PartialEnvironment;
