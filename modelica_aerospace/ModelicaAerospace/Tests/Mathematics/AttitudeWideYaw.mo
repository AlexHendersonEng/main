within ModelicaAerospace.Tests.Mathematics;
model AttitudeWideYaw "Large-yaw attitude validation case"
  extends ModelicaAerospace.Tests.Mathematics.AttitudeValidation(
    roll=-0.4,
    pitch=0.25,
    yaw=2.6);
end AttitudeWideYaw;
