within ModelicaAerospace.Types;
record WindState "Steady and turbulent wind components"
  ModelicaAerospace.Types.Velocity velocityNED[3] = {0, 0, 0} "Steady wind in NED";
  ModelicaAerospace.Types.Velocity turbulenceBody[3] = {0, 0, 0}
    "Turbulence velocity in body axes";
  ModelicaAerospace.Types.AngularVelocity turbulenceRatesBody[3] = {0, 0, 0}
    "Turbulence angular rates in body axes";
end WindState;
