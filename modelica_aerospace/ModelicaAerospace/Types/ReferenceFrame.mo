within ModelicaAerospace.Types;
type ReferenceFrame = enumeration(
  Body "Vehicle body: x forward, y starboard, z down",
  NED "Local navigation: north, east, down",
  ECEF "Earth-centered, Earth-fixed",
  ECI "Earth-centered inertial") "Supported reference-frame identifiers";
