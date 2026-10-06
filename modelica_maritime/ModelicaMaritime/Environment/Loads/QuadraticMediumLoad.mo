within ModelicaMaritime.Environment.Loads;
block QuadraticMediumLoad "Current- or wind-relative quadratic body load"
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter Real dragCoefficient[3] = {1, 1, 1};
  parameter Real projectedArea[3](each unit="m2") = {1, 1, 1};
  parameter ModelicaMaritime.Types.Vector3 applicationPointBody = {0, 0, 0};
  ModelicaMaritime.Interfaces.Vector3Input relativeVelocityBody(each unit="m/s")
    "Vehicle velocity minus current or wind velocity in body axes";
  ModelicaMaritime.Interfaces.Vector6Output generalizedLoadBody;
equation
  generalizedLoadBody =
    ModelicaMaritime.Environment.Loads.quadraticDragLoad(
      relativeVelocityBody,
      density,
      dragCoefficient,
      projectedArea,
      applicationPointBody);
end QuadraticMediumLoad;
