within ModelicaAerospace.Types;
record RotationalState "Body attitude and angular velocity"
  ModelicaAerospace.Types.Quaternion quaternion = {1, 0, 0, 0}
    "Scalar-first active rotation from body coordinates to the owning reference frame";
  ModelicaAerospace.Types.AngularVelocity angularVelocityBody[3] = {0, 0, 0}
    "Body angular velocity {p, q, r}";
end RotationalState;
