model MassSpringDamper "Mass on a spring and damper, driven by an optional force"
  parameter Real m = 1.0 "Mass [kg]";
  parameter Real k = 20.0 "Spring stiffness [N/m]";
  parameter Real c = 0.5 "Damping coefficient [N.s/m]";
  Real x(start = 1.0, fixed = true) "Position [m]";
  Real v(start = 0.0, fixed = true) "Velocity [m/s]";
equation
  der(x) = v;
  m * der(v) = -k * x - c * v;
end MassSpringDamper;
