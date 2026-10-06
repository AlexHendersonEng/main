within ModelicaMaritime.Coordinates;
function quaternionDerivativeBodyToNED
  "Quaternion derivative for body angular velocity and scalar-first active rotation"
  input ModelicaMaritime.Types.Quaternion quaternion;
  input ModelicaMaritime.Types.AngularVelocity angularVelocityBody[3];
  output Real derivative[4](each unit="1/s");
algorithm
  derivative := 0.5 * {
    -quaternion[2] * angularVelocityBody[1]
      - quaternion[3] * angularVelocityBody[2]
      - quaternion[4] * angularVelocityBody[3],
    quaternion[1] * angularVelocityBody[1]
      + quaternion[3] * angularVelocityBody[3]
      - quaternion[4] * angularVelocityBody[2],
    quaternion[1] * angularVelocityBody[2]
      + quaternion[4] * angularVelocityBody[1]
      - quaternion[2] * angularVelocityBody[3],
    quaternion[1] * angularVelocityBody[3]
      + quaternion[2] * angularVelocityBody[2]
      - quaternion[3] * angularVelocityBody[1]};
end quaternionDerivativeBodyToNED;
