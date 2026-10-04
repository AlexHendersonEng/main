within ModelicaAerospace.Types;
record SphericalInitialState "Initial rotating-Earth rigid-body state"
  ModelicaAerospace.Types.Length positionECEF[3] = {6378137, 0, 0};
  ModelicaAerospace.Types.Velocity velocityECEF[3] = {0, 0, 0};
  ModelicaAerospace.Types.Quaternion quaternionBodyToECEF = {
    0.7071067811865476,
    0,
    -0.7071067811865476,
    0};
  ModelicaAerospace.Types.AngularVelocity angularVelocityBody[3] = {0, 0, 0};
end SphericalInitialState;
