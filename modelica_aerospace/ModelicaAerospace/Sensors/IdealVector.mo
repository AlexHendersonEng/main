within ModelicaAerospace.Sensors;
block IdealVector "Ideal three-axis sensor"
  extends ModelicaAerospace.Interfaces.PartialSensor;
equation
  measurement = truth;
end IdealVector;
