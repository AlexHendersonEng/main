within ModelicaMaritime.Tests.Common;
block EnvironmentStub "Concrete common-environment interface check"
  extends ModelicaMaritime.Interfaces.PartialEnvironment;
equation
  temperature = 288.15;
  pressure = ModelicaMaritime.Constants.standardAtmosphericPressure
    + density * ModelicaMaritime.Constants.standardGravity * depth;
  density = ModelicaMaritime.Constants.standardSeawaterDensity;
  currentVelocityNED = {1, 0, 0};
end EnvironmentStub;
