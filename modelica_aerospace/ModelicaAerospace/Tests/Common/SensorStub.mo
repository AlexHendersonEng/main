within ModelicaAerospace.Tests.Common;
block SensorStub "Concrete sensor interface"
  extends ModelicaAerospace.Interfaces.PartialSensor;
equation
  measurement = truth;
end SensorStub;
