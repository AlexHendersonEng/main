within ModelicaMaritime.Sensors;
block IdealScalar "Ideal scalar sensor"
  extends ModelicaMaritime.Interfaces.PartialSensor;
equation
  measurement = truth;
end IdealScalar;
