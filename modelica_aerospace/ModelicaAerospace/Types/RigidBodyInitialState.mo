within ModelicaAerospace.Types;
record RigidBodyInitialState "Initial flat-Earth rigid-body state"
  ModelicaAerospace.Types.Length positionNED[3] = {0, 0, 0};
  ModelicaAerospace.Types.Velocity velocityBody[3] = {0, 0, 0};
  ModelicaAerospace.Types.Quaternion quaternionBodyToNED = {1, 0, 0, 0};
  ModelicaAerospace.Types.AngularVelocity angularVelocityBody[3] = {0, 0, 0};
end RigidBodyInitialState;
