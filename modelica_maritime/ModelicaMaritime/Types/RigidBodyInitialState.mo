within ModelicaMaritime.Types;
record RigidBodyInitialState "Initial six-degree-of-freedom vehicle state"
  ModelicaMaritime.Types.Length positionNED[3] = {0, 0, 0};
  ModelicaMaritime.Types.Velocity velocityBody[3] = {0, 0, 0};
  ModelicaMaritime.Types.Quaternion quaternionBodyToNED = {1, 0, 0, 0};
  ModelicaMaritime.Types.AngularVelocity angularVelocityBody[3] = {0, 0, 0};
end RigidBodyInitialState;
