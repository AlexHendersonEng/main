within ModelicaMaritime.Tests.Common;
block SensorStub "Concrete common-sensor interface check"
  extends ModelicaMaritime.Interfaces.PartialSensor;
equation
  measurement = truth;
end SensorStub;
