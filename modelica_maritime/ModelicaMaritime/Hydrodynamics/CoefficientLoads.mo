within ModelicaMaritime.Hydrodynamics;
block CoefficientLoads "Planar force and yaw-moment coefficient scaling"
  parameter ModelicaMaritime.Types.Density density =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter ModelicaMaritime.Types.Length referenceLength = 1;
  parameter ModelicaMaritime.Types.Length referenceDraft = 1;
  ModelicaMaritime.Interfaces.Vector3Input coefficients "{CX, CY, CN}";
  ModelicaMaritime.Interfaces.Vector3Input relativeVelocityBody
    "Relative planar velocity {u_r, v_r, r}";
  ModelicaMaritime.Interfaces.Vector3Output generalizedLoad
    "Body load {X, Y, N}";
protected
  Real relativeSpeed(unit="m/s");
equation
  relativeSpeed = sqrt(
    relativeVelocityBody[1] * relativeVelocityBody[1]
    + relativeVelocityBody[2] * relativeVelocityBody[2]);
  generalizedLoad = ModelicaMaritime.Hydrodynamics.coefficientPlanarLoads(
    coefficients,
    density,
    referenceLength,
    referenceDraft,
    relativeSpeed);
end CoefficientLoads;
