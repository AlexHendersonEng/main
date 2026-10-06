within ModelicaMaritime.Interfaces;
partial block PartialPlanarVehicle "Common surge, sway, and yaw vehicle interface"
  ModelicaMaritime.Interfaces.Vector3Input generalizedForceBody
    "Body load {X, Y, N}; forces in N and yaw moment in N.m";
  ModelicaMaritime.Interfaces.Vector3Output poseNED
    "Planar pose {north, east, heading}; position in m and heading in rad";
  ModelicaMaritime.Interfaces.Vector3Output velocityBody
    "Planar body velocity {u, v, r}; translation in m/s and yaw rate in rad/s";
end PartialPlanarVehicle;
