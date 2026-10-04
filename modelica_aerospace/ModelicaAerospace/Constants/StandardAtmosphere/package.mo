within ModelicaAerospace.Constants;
package StandardAtmosphere "1976 standard-atmosphere reference constants"
  constant ModelicaAerospace.Types.Temperature seaLevelTemperature = 288.15;
  constant ModelicaAerospace.Types.Pressure seaLevelPressure = 101325;
  constant ModelicaAerospace.Types.Density seaLevelDensity = 1.225;
  constant ModelicaAerospace.Types.Acceleration standardGravity = 9.80665;
  constant Real specificGasConstant(unit="J/(kg.K)") = 287.05287;
  constant Real heatCapacityRatio = 1.4;
  constant ModelicaAerospace.Types.Length geopotentialRadius = 6356766;
  annotation (Documentation(info="<html>
<p>Reference values used by the U.S. Standard Atmosphere 1976.</p>
</html>"));
end StandardAtmosphere;
