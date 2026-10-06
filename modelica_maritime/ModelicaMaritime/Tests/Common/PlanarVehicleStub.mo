within ModelicaMaritime.Tests.Common;
block PlanarVehicleStub "Concrete planar-vehicle interface check"
  extends ModelicaMaritime.Interfaces.PartialPlanarVehicle;
equation
  poseNED = {0, 0, 0};
  velocityBody = generalizedForceBody;
end PlanarVehicleStub;
