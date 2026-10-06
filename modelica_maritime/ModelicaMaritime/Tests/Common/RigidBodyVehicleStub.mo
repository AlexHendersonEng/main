within ModelicaMaritime.Tests.Common;
block RigidBodyVehicleStub "Concrete rigid-body vehicle interface check"
  extends ModelicaMaritime.Interfaces.PartialRigidBodyVehicle;
equation
  positionNED = {0, 0, 0};
  velocityBody = generalizedForceBody;
  quaternionBodyToNED = {1, 0, 0, 0};
end RigidBodyVehicleStub;
