within ModelicaAutomotive.Tests.Common;
model CommonValidation "Validate common records, constants, interfaces, and utilities"
  ModelicaAutomotive.Types.VehicleGeometry geometry;
  ModelicaAutomotive.Types.MassProperties properties;
  ModelicaAutomotive.Types.VehicleInitialState initialState;
  ModelicaAutomotive.Tests.Common.VehicleStub vehicle;
  output Real vehicleVelocity[3];
  output Real angularVelocity[3];
  output Real gravity;
  output Real wheelbase;
  output Real quaternionScalar;
  output Real checkedPositive;
equation
  vehicle.forceBody = {7, 8, 9};
  vehicle.momentBody = {0.1, 0.2, 0.3};
  vehicleVelocity = vehicle.velocityBody;
  angularVelocity = vehicle.angularVelocityBody;
  gravity = ModelicaAutomotive.Constants.standardGravity;
  wheelbase = geometry.wheelbase;
  quaternionScalar = initialState.quaternionBodyToWorld[1];
  assert(properties.mass > 0, "mass must be positive");
  checkedPositive = properties.mass;
end CommonValidation;
