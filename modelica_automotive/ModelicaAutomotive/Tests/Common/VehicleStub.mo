within ModelicaAutomotive.Tests.Common;
block VehicleStub "Concrete common-interface validation stub"
  extends ModelicaAutomotive.Interfaces.PartialVehicleDynamics;
equation
  positionWorld = {0, 0, 0};
  velocityBody = forceBody;
  quaternionBodyToWorld = {1, 0, 0, 0};
  angularVelocityBody = momentBody;
end VehicleStub;
