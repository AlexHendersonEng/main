within ModelicaAutomotive.Mathematics;
function quaternionDerivative
  "Quaternion derivative for body angular velocity {p, q, r}"
  input ModelicaAutomotive.Types.Quaternion quaternion;
  input ModelicaAutomotive.Types.AngularVelocity angularVelocityBody[3];
  output Real derivative[4](each unit="1/s");
protected
  ModelicaAutomotive.Types.Quaternion omegaQuaternion;
algorithm
  omegaQuaternion := {
    0,
    angularVelocityBody[1],
    angularVelocityBody[2],
    angularVelocityBody[3]};
  derivative := 0.5 * ModelicaAutomotive.Mathematics.quaternionMultiply(
    quaternion,
    omegaQuaternion);
end quaternionDerivative;
