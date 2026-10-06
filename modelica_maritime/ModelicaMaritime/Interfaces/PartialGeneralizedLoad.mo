within ModelicaMaritime.Interfaces;
partial block PartialGeneralizedLoad "Common hydrodynamic or actuator load interface"
  ModelicaMaritime.Interfaces.Vector6Input relativeVelocityBody
    "Water- or air-relative body velocity {u, v, w, p, q, r}";
  ModelicaMaritime.Interfaces.Vector6Output generalizedLoadBody
    "Body load {X, Y, Z, K, M, N}";
end PartialGeneralizedLoad;
