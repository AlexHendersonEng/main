within ModelicaAutomotive.Types;
record VehicleInitialState "Initial rigid vehicle state"
  ModelicaAutomotive.Types.Length positionWorld[3] = {0, 0, 0};
  ModelicaAutomotive.Types.Velocity velocityBody[3] = {0, 0, 0};
  ModelicaAutomotive.Types.Quaternion quaternionBodyToWorld = {1, 0, 0, 0};
  ModelicaAutomotive.Types.AngularVelocity angularVelocityBody[3] = {0, 0, 0};
end VehicleInitialState;
