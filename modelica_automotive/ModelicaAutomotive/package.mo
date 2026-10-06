within ;
package ModelicaAutomotive
  "Composable automotive vehicle-dynamics blocks"

  annotation (
    version="0.1.0",
    uses(Modelica(version="4.0.0")),
    Documentation(info="<html>
<p>ModelicaAutomotive provides SI-unit, signal-oriented building blocks for
automotive vehicle-dynamics simulation using only the Modelica Standard
Library. Vehicle body axes are x forward, y left, z upward. World axes are
right-handed with Z upward. Angles are radians, quaternions are scalar-first
active body-to-world rotations, positive pitch is nose-down, and wheel arrays are ordered front-left,
front-right, rear-left, rear-right.</p>
</html>"));
end ModelicaAutomotive;
