within ModelicaMaritime.Tests.VesselDynamics;
model CoefficientTableValidation "Validate MSL-backed planar coefficient interpolation"
  ModelicaMaritime.Hydrodynamics.TablePlanarCoefficients coefficients(
    tableX=[-1, -0.2; 0, 0; 1, -0.2],
    tableY=[-1, 0.5; 0, 0; 1, -0.5],
    tableN=[-1, 0.1; 0, 0; 1, -0.1]);
  output Real interpolated[3];
equation
  coefficients.abscissa = 0.5;
  interpolated = coefficients.coefficients + time * {1e-7, 2e-7, 3e-7};
end CoefficientTableValidation;
