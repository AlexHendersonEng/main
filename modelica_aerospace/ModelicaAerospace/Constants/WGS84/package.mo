within ModelicaAerospace.Constants;
package WGS84 "WGS-84 defining and derived constants"
  constant ModelicaAerospace.Types.Length semiMajorAxis = 6378137.0 "Equatorial radius";
  constant Real flattening = 1 / 298.257223563 "Geometric flattening";
  constant ModelicaAerospace.Types.Length semiMinorAxis = semiMajorAxis * (1 - flattening)
    "Polar semi-minor axis";
  constant Real firstEccentricitySquared = flattening * (2 - flattening);
  constant ModelicaAerospace.Types.AngularVelocity earthRotationRate = 7.292115e-5;
  constant Real gravitationalParameter(unit="m3/s2") = 3.986004418e14;
  annotation (Documentation(info="<html>
<p>Constants use the WGS-84 defining values. Coordinates use east-positive
longitude and ellipsoidal altitude.</p>
</html>"));
end WGS84;
