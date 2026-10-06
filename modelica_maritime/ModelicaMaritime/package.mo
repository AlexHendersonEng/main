within ;
package ModelicaMaritime
  "Composable maritime system dynamics"

  annotation (
    version="0.1.0",
    uses(Modelica(version="4.0.0")),
    Documentation(info="<html>
<p>ModelicaMaritime provides SI-unit, signal-oriented building blocks for
surface-vessel and underwater-vehicle simulation using only the Modelica
Standard Library. Body axes are x forward, y starboard, z down; local
navigation uses north, east, down; angles are radians; and quaternions are
scalar-first active rotations. The package includes shared mathematics and
coordinates, environment and bathymetry, generic/MMG/Fossen vessel dynamics,
propulsion and actuators, sensors and GNC, packaged validation models, and
finite runnable examples.</p>
</html>"));
end ModelicaMaritime;
