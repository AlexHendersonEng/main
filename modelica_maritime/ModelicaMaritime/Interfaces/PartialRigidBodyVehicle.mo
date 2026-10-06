within ModelicaMaritime.Interfaces;
partial block PartialRigidBodyVehicle "Common six-degree-of-freedom vehicle interface"
  ModelicaMaritime.Interfaces.Vector6Input generalizedForceBody
    "Body load {X, Y, Z, K, M, N}";
  ModelicaMaritime.Interfaces.Vector3Output positionNED(each unit="m")
    "North, east, down position";
  ModelicaMaritime.Interfaces.Vector6Output velocityBody
    "Body velocity {u, v, w, p, q, r}";
  ModelicaMaritime.Interfaces.QuaternionOutput quaternionBodyToNED
    "Active body-to-NED rotation";
end PartialRigidBodyVehicle;
