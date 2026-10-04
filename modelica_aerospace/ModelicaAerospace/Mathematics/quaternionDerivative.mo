within ModelicaAerospace.Mathematics;
function quaternionDerivative
  "Quaternion derivative for body angular velocity {p, q, r}"
  input ModelicaAerospace.Types.Quaternion quaternion
    "Active body-to-reference rotation";
  input ModelicaAerospace.Types.AngularVelocity angularVelocityBody[3];
  output Real derivative[4](each unit="1/s");
protected
  ModelicaAerospace.Types.Quaternion omegaQuaternion;
algorithm
  omegaQuaternion := {
    0,
    angularVelocityBody[1],
    angularVelocityBody[2],
    angularVelocityBody[3]};
  derivative := 0.5 * ModelicaAerospace.Mathematics.quaternionMultiply(
    quaternion,
    omegaQuaternion);
end quaternionDerivative;
