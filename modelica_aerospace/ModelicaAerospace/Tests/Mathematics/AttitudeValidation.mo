within ModelicaAerospace.Tests.Mathematics;
model AttitudeValidation "Exercise vector, matrix, quaternion, and Euler primitives"
  parameter Real roll = 0.3;
  parameter Real pitch = -0.2;
  parameter Real yaw = 0.7;
  parameter Real p = 0.1;
  parameter Real qRate = -0.2;
  parameter Real r = 0.3;
  ModelicaAerospace.Mathematics.Blocks.Euler321ToQuaternion eulerBlock;
  ModelicaAerospace.Mathematics.Blocks.QuaternionToEuler321 quaternionBlock;
  ModelicaAerospace.Mathematics.Blocks.QuaternionToDCM dcmBlock;
  ModelicaAerospace.Mathematics.Blocks.RotateVectorByQuaternion rotateBlock;
  output Real quaternion[4];
  output Real dcm[3, 3];
  output Real eulerRoundTrip[3];
  output Real quaternionRoundTrip[4];
  output Real quaternionDerivative[4];
  output Real conjugateProduct[4];
  output Real rotatedVector[3];
  output Real crossResult[3];
  output Real normalizedVector[3];
  output Real skew[3, 3];
  output Real wrappedAngle;
  output Real determinant;
  output Real rotationValid;
  output Real invalidRotationValid;
  output Real blockQuaternion[4];
  output Real blockEuler[3];
  output Real blockDCM[3, 3];
  output Real blockRotatedVector[3];
equation
  quaternion = ModelicaAerospace.Mathematics.euler321ToQuaternion(roll, pitch, yaw);
  dcm = ModelicaAerospace.Mathematics.quaternionToDCM(quaternion);
  eulerRoundTrip = ModelicaAerospace.Mathematics.quaternionToEuler321(quaternion);
  quaternionRoundTrip = ModelicaAerospace.Mathematics.dcmToQuaternion(dcm);
  quaternionDerivative = ModelicaAerospace.Mathematics.quaternionDerivative(
    quaternion,
    {p, qRate, r});
  conjugateProduct = ModelicaAerospace.Mathematics.quaternionMultiply(
    quaternion,
    ModelicaAerospace.Mathematics.quaternionConjugate(quaternion));
  rotatedVector = ModelicaAerospace.Mathematics.rotateVector(quaternion, {1, 0, 0});
  crossResult = ModelicaAerospace.Mathematics.cross3({1, 0, 0}, {0, 1, 0});
  normalizedVector = ModelicaAerospace.Mathematics.normalizeVector3({3, 4, 0});
  skew = ModelicaAerospace.Mathematics.skewMatrix({1, 2, 3});
  wrappedAngle =
    ModelicaAerospace.Mathematics.wrapAngle(yaw + 12.56637061435917 + 0.1 * time);
  determinant = ModelicaAerospace.Mathematics.determinant3(dcm);
  rotationValid =
    if ModelicaAerospace.Mathematics.isRotationMatrix(dcm) then 1 else 0;
  invalidRotationValid =
    if ModelicaAerospace.Mathematics.isRotationMatrix(
      [1, 0, 0; 0, 1, 0; 0, 0, -1]) then 1 else 0;

  eulerBlock.roll = roll;
  eulerBlock.pitch = pitch;
  eulerBlock.yaw = yaw;
  blockQuaternion = eulerBlock.quaternion;

  quaternionBlock.quaternion = quaternion;
  blockEuler = quaternionBlock.euler;

  dcmBlock.quaternion = quaternion;
  blockDCM = dcmBlock.dcm;

  rotateBlock.quaternion = quaternion;
  rotateBlock.vector = {1, 0, 0};
  blockRotatedVector = rotateBlock.rotated;
end AttitudeValidation;
