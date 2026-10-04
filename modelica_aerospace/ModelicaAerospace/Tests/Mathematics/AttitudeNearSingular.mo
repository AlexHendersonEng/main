within ModelicaAerospace.Tests.Mathematics;
model AttitudeNearSingular "Near-gimbal-lock attitude validation case"
  extends ModelicaAerospace.Tests.Mathematics.AttitudeValidation(
    roll=0.3,
    pitch=1.570796226794897,
    yaw=-0.8);
end AttitudeNearSingular;
