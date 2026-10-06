within ModelicaMaritime.Interfaces;
partial block PartialSensor "Common scalar sensor interface"
  ModelicaMaritime.Interfaces.RealInput truth "True input quantity";
  ModelicaMaritime.Interfaces.RealOutput measurement "Measured output quantity";
end PartialSensor;
