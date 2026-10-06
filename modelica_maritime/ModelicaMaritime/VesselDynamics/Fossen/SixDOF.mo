within ModelicaMaritime.VesselDynamics.Fossen;
block SixDOF "Fossen-style 6-DoF surface and underwater vehicle dynamics"
  extends ModelicaMaritime.Interfaces.PartialRigidBodyVehicle;
  parameter ModelicaMaritime.Types.MassProperties massProperties;
  parameter ModelicaMaritime.Types.FossenHydrodynamicProperties hydrodynamics;
  parameter ModelicaMaritime.Types.RigidBodyInitialState initialState;
  parameter ModelicaMaritime.Types.Density waterDensity =
    ModelicaMaritime.Constants.standardSeawaterDensity;
  parameter Real gravity(unit="m/s2") =
    ModelicaMaritime.Constants.standardGravity;
  parameter Real quaternionStabilization(unit="1/s") = 20;
  ModelicaMaritime.Interfaces.Vector3Input currentVelocityNED(each unit="m/s");
  ModelicaMaritime.Interfaces.Vector6Output relativeVelocityBody;
  ModelicaMaritime.Interfaces.Vector6Output accelerationBody;
  ModelicaMaritime.Interfaces.Vector6Output restoringLoadBody;
  ModelicaMaritime.Interfaces.Vector6Output dampingLoadBody;
  ModelicaMaritime.Interfaces.Vector3Output velocityNED(each unit="m/s");
  output Real rotationBodyToNED[3, 3];
  ModelicaMaritime.Interfaces.RealOutput quaternionNorm;
  ModelicaMaritime.Interfaces.RealOutput kineticEnergy(unit="J");
  ModelicaMaritime.Interfaces.RealOutput dissipationPower(unit="W");
protected
  Real positionState[3](start=initialState.positionNED, each fixed=true);
  Real velocityState[6](
    start={
      initialState.velocityBody[1],
      initialState.velocityBody[2],
      initialState.velocityBody[3],
      initialState.angularVelocityBody[1],
      initialState.angularVelocityBody[2],
      initialState.angularVelocityBody[3]},
    each fixed=true);
  Real quaternionState[4](
    start=initialState.quaternionBodyToNED,
    each fixed=true);
  Real normalizedQuaternion[4];
  Real currentVelocityBody[3];
  Real quaternionRate[4];
  Real rigidBodyMass[6, 6];
  Real totalMass[6, 6];
  Real rigidBodyCoriolis[6, 6];
  Real addedMassCoriolis[6, 6];
  Real rightHandSide[6];
  Real rowOffDiagonal[6];
equation
  rigidBodyMass =
    ModelicaMaritime.VesselDynamics.Fossen.rigidBodyMassMatrix(massProperties);
  totalMass = rigidBodyMass + hydrodynamics.addedMass;
  for row in 1:6 loop
    for column in 1:6 loop
      assert(
        abs(totalMass[row, column] - totalMass[column, row]) < 1e-10,
        "Total 6-DoF inertia matrix must be symmetric");
    end for;
    rowOffDiagonal[row] =
      abs(totalMass[row, 1])
      + abs(totalMass[row, 2])
      + abs(totalMass[row, 3])
      + abs(totalMass[row, 4])
      + abs(totalMass[row, 5])
      + abs(totalMass[row, 6])
      - abs(totalMass[row, row]);
    assert(
      totalMass[row, row] > rowOffDiagonal[row],
      "Total 6-DoF inertia matrix must be strictly diagonally dominant and positive");
    assert(
      hydrodynamics.linearDamping[row, row] >= 0
        and hydrodynamics.quadraticDamping[row, row] >= 0,
      "6-DoF damping diagonal terms must be non-negative");
  end for;
  quaternionNorm = sqrt(
    quaternionState[1] * quaternionState[1]
    + quaternionState[2] * quaternionState[2]
    + quaternionState[3] * quaternionState[3]
    + quaternionState[4] * quaternionState[4]);
  assert(quaternionNorm > 0, "Quaternion norm must be positive");
  normalizedQuaternion = quaternionState / quaternionNorm;
  rotationBodyToNED =
    ModelicaMaritime.Coordinates.quaternionBodyToNEDMatrix(normalizedQuaternion);
  velocityNED = rotationBodyToNED * velocityState[1:3];
  currentVelocityBody = transpose(rotationBodyToNED) * currentVelocityNED;
  relativeVelocityBody = {
    velocityState[1] - currentVelocityBody[1],
    velocityState[2] - currentVelocityBody[2],
    velocityState[3] - currentVelocityBody[3],
    velocityState[4],
    velocityState[5],
    velocityState[6]};
  rigidBodyCoriolis =
    ModelicaMaritime.VesselDynamics.Fossen.coriolisMatrix(
      rigidBodyMass,
      velocityState);
  addedMassCoriolis =
    ModelicaMaritime.VesselDynamics.Fossen.coriolisMatrix(
      hydrodynamics.addedMass,
      relativeVelocityBody);
  dampingLoadBody =
    ModelicaMaritime.VesselDynamics.Fossen.dampingLoad(
      hydrodynamics.linearDamping,
      hydrodynamics.quadraticDamping,
      relativeVelocityBody);
  restoringLoadBody =
    ModelicaMaritime.VesselDynamics.Fossen.hydrostaticRestoringLoad(
      massProperties,
      normalizedQuaternion,
      waterDensity,
      gravity);
  rightHandSide = generalizedForceBody
    + restoringLoadBody
    + dampingLoadBody
    - rigidBodyCoriolis * velocityState
    - addedMassCoriolis * relativeVelocityBody;
  totalMass * accelerationBody = rightHandSide;
  der(positionState) = velocityNED;
  der(velocityState) = accelerationBody;
  quaternionRate =
    ModelicaMaritime.Coordinates.quaternionDerivativeBodyToNED(
      normalizedQuaternion,
      velocityState[4:6]);
  der(quaternionState) = quaternionRate
    + quaternionStabilization
      * (1 - quaternionNorm * quaternionNorm) * quaternionState;
  positionNED = positionState;
  velocityBody = velocityState;
  quaternionBodyToNED = normalizedQuaternion;
  kineticEnergy = 0.5 * velocityState * (rigidBodyMass * velocityState)
    + 0.5 * relativeVelocityBody
      * (hydrodynamics.addedMass * relativeVelocityBody);
  dissipationPower = -relativeVelocityBody * dampingLoadBody;
end SixDOF;
