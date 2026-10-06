within ModelicaMaritime.VesselDynamics.Generic;
block Planar3DOF "Generic surge, sway, and yaw maneuvering dynamics"
  extends ModelicaMaritime.Interfaces.PartialPlanarVehicle;
  parameter ModelicaMaritime.Types.PlanarMassProperties massProperties;
  parameter ModelicaMaritime.Types.PlanarHydrodynamicProperties hydrodynamics;
  parameter ModelicaMaritime.Types.PlanarInitialState initialState;
  ModelicaMaritime.Interfaces.Vector3Input currentVelocityNED(each unit="m/s")
    "Water-current velocity in NED; vertical component is ignored";
  ModelicaMaritime.Interfaces.Vector3Output relativeVelocityBody
    "Water-relative planar velocity {u_r, v_r, r}";
  ModelicaMaritime.Interfaces.Vector3Output accelerationBody
    "Body acceleration {du/dt, dv/dt, dr/dt}";
  ModelicaMaritime.Interfaces.Vector3Output currentVelocityBody
    "Current expressed in planar body axes {u_c, v_c, 0}";
  ModelicaMaritime.Interfaces.RealOutput kineticEnergy(unit="J");
  ModelicaMaritime.Interfaces.RealOutput dissipationPower(unit="W");
protected
  Real poseState[3](
    start={initialState.positionNED[1], initialState.positionNED[2], initialState.heading},
    each fixed=true);
  Real velocityState[3](
    start={initialState.velocityBody[1], initialState.velocityBody[2], initialState.yawRate},
    each fixed=true);
  Real rigidBodyMass[3, 3];
  Real totalMass[3, 3];
  Real rigidBodyCoriolis[3, 3];
  Real addedMassCoriolis[3, 3];
  Real dampingLoad[3];
  Real rightHandSide[3];
  Real determinant2;
  Real determinant3;
equation
  rigidBodyMass = ModelicaMaritime.Hydrodynamics.planarRigidBodyMassMatrix(
    massProperties);
  totalMass = rigidBodyMass + hydrodynamics.addedMass;
  determinant2 = totalMass[1, 1] * totalMass[2, 2]
    - totalMass[1, 2] * totalMass[2, 1];
  determinant3 =
    totalMass[1, 1] * (
      totalMass[2, 2] * totalMass[3, 3] - totalMass[2, 3] * totalMass[3, 2])
    - totalMass[1, 2] * (
      totalMass[2, 1] * totalMass[3, 3] - totalMass[2, 3] * totalMass[3, 1])
    + totalMass[1, 3] * (
      totalMass[2, 1] * totalMass[3, 2] - totalMass[2, 2] * totalMass[3, 1]);
  assert(
    abs(hydrodynamics.addedMass[1, 2] - hydrodynamics.addedMass[2, 1]) < 1e-10
      and abs(hydrodynamics.addedMass[1, 3] - hydrodynamics.addedMass[3, 1]) < 1e-10
      and abs(hydrodynamics.addedMass[2, 3] - hydrodynamics.addedMass[3, 2]) < 1e-10,
    "Planar added-mass matrix must be symmetric");
  assert(
    totalMass[1, 1] > 0 and determinant2 > 0 and determinant3 > 0,
    "Total planar inertia matrix must be symmetric positive definite");
  assert(
    hydrodynamics.linearDamping[1, 1] >= 0
      and hydrodynamics.linearDamping[2, 2] >= 0
      and hydrodynamics.linearDamping[3, 3] >= 0
      and hydrodynamics.quadraticDamping[1, 1] >= 0
      and hydrodynamics.quadraticDamping[2, 2] >= 0
      and hydrodynamics.quadraticDamping[3, 3] >= 0,
    "Planar damping diagonal terms must be non-negative");
  currentVelocityBody = {
    cos(poseState[3]) * currentVelocityNED[1]
      + sin(poseState[3]) * currentVelocityNED[2],
    -sin(poseState[3]) * currentVelocityNED[1]
      + cos(poseState[3]) * currentVelocityNED[2],
    0};
  relativeVelocityBody = velocityState - currentVelocityBody;
  rigidBodyCoriolis = ModelicaMaritime.Hydrodynamics.planarCoriolisMatrix(
    rigidBodyMass,
    velocityState);
  addedMassCoriolis = ModelicaMaritime.Hydrodynamics.planarCoriolisMatrix(
    hydrodynamics.addedMass,
    relativeVelocityBody);
  dampingLoad = ModelicaMaritime.Hydrodynamics.planarDampingLoad(
    hydrodynamics.linearDamping,
    hydrodynamics.quadraticDamping,
    relativeVelocityBody);
  rightHandSide = generalizedForceBody
    + dampingLoad
    - rigidBodyCoriolis * velocityState
    - addedMassCoriolis * relativeVelocityBody;
  accelerationBody = ModelicaMaritime.Mathematics.solveLinear3(
    totalMass,
    rightHandSide);
  der(poseState) = ModelicaMaritime.Coordinates.planarKinematics(
    poseState[3],
    velocityState);
  der(velocityState) = accelerationBody;
  poseNED = poseState;
  velocityBody = velocityState;
  kineticEnergy = 0.5 * velocityState * (rigidBodyMass * velocityState)
    + 0.5 * relativeVelocityBody
      * (hydrodynamics.addedMass * relativeVelocityBody);
  dissipationPower = -relativeVelocityBody * dampingLoad;
end Planar3DOF;
