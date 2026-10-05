within ModelicaAutomotive.Interfaces;
partial block PartialVehicleDynamics "Common rigid-vehicle force and state interface"
  ModelicaAutomotive.Interfaces.Vector3Input forceBody(each unit="N")
    "Applied force in body axes";
  ModelicaAutomotive.Interfaces.Vector3Input momentBody(each unit="N.m")
    "Applied moment in body axes";
  ModelicaAutomotive.Interfaces.Vector3Output positionWorld(each unit="m")
    "World-frame position";
  ModelicaAutomotive.Interfaces.Vector3Output velocityBody(each unit="m/s")
    "Body-axis velocity";
  ModelicaAutomotive.Interfaces.QuaternionOutput quaternionBodyToWorld
    "Active body-to-world rotation";
  ModelicaAutomotive.Interfaces.Vector3Output angularVelocityBody(each unit="rad/s")
    "Body rates {p, q, r}";
end PartialVehicleDynamics;
