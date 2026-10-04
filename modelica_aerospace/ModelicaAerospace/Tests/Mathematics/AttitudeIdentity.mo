within ModelicaAerospace.Tests.Mathematics;
model AttitudeIdentity "Identity attitude validation case"
  extends ModelicaAerospace.Tests.Mathematics.AttitudeValidation(
    roll=0,
    pitch=0,
    yaw=0);
end AttitudeIdentity;
