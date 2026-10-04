within ModelicaAerospace.Interfaces;
partial block PartialVehicleDynamics "Common body-force vehicle-dynamics interface"
  ModelicaAerospace.Interfaces.Vector3Input forceBody(each unit="N")
    "Applied body-axis force";
  ModelicaAerospace.Interfaces.Vector3Input momentBody(each unit="N.m")
    "Applied body-axis moment";
  ModelicaAerospace.Interfaces.Vector3Output positionNED(each unit="m")
    "North, east, down position";
  ModelicaAerospace.Interfaces.Vector3Output velocityBody(each unit="m/s")
    "Body-axis velocity";
  ModelicaAerospace.Interfaces.QuaternionOutput quaternionBodyToNED
    "Active body-to-NED rotation";
  ModelicaAerospace.Interfaces.Vector3Output angularVelocityBody(each unit="rad/s")
    "Body rates {p, q, r}";
end PartialVehicleDynamics;
