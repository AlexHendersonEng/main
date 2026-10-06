within ModelicaMaritime;
package Constants "Physical and numerical constants"
  constant Real standardGravity(unit="m/s2") = 9.80665
    "Conventional standard gravitational acceleration";
  constant Real standardSeawaterDensity(unit="kg/m3") = 1025
    "Nominal seawater density";
  constant Real standardAtmosphericPressure(unit="Pa") = 101325
    "Standard atmospheric pressure";
  constant Real small = 1e-12 "Small positive numerical threshold";
  annotation (Documentation(info="<html>
<p>Shared physical reference values and numerical thresholds.</p>
</html>"));
end Constants;
