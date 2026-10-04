within ModelicaAerospace.Tests.Common;
block VehicleStub "Concrete vehicle-dynamics interface"
  extends ModelicaAerospace.Interfaces.PartialVehicleDynamics;
equation
  positionNED = {0, 0, 0};
  velocityBody = forceBody;
  quaternionBodyToNED = {1, 0, 0, 0};
  angularVelocityBody = momentBody;
end VehicleStub;
