within ;
package ModelicaAerospace
  "Composable aerospace simulation blocks"

  annotation (
    version="0.1.0",
    uses(Modelica(version="4.0.0")),
    Documentation(info="<html>
<p>ModelicaAerospace provides SI-unit, signal-oriented building blocks for
aerospace simulation using only the Modelica Standard Library. Body axes are
x forward, y starboard, z down; local navigation uses north, east, down;
angles are radians; and quaternions are scalar-first active rotations.
The package includes mathematics, coordinates, environment, flight dynamics,
vehicle subsystems, GNC blocks, packaged validation models, and runnable
integrated examples.</p>
</html>"));
end ModelicaAerospace;
