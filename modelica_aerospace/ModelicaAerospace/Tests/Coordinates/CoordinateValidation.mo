within ModelicaAerospace.Tests.Coordinates;
model CoordinateValidation "Exercise WGS-84 and rotating-Earth transformations"
  parameter Real latitude = 0.7;
  parameter Real longitude = -2.4;
  parameter Real altitude = 1234;
  parameter Real testTime = 4321;
  ModelicaAerospace.Types.GeodeticPosition geodetic;
  ModelicaAerospace.Types.GeodeticPosition geodeticRoundTrip;
  ModelicaAerospace.Types.GeodeticPosition localGeodetic;
  ModelicaAerospace.Coordinates.Blocks.GeodeticToECEF geodeticBlock;
  ModelicaAerospace.Coordinates.Blocks.ECEFToGeodetic ecefBlock;
  ModelicaAerospace.Coordinates.Blocks.NEDToECEFVector nedBlock;
  ModelicaAerospace.Coordinates.Blocks.ECEFToNEDVector ecefVectorBlock;
  output Real ecef[3];
  output Real roundTrip[3];
  output Real nedToECEF[3, 3];
  output Real ecefToNED[3, 3];
  output Real nedVector[3];
  output Real ecefVector[3];
  output Real nedRoundTrip[3];
  output Real localNEDRoundTrip[3];
  output Real eciPosition[3];
  output Real ecefPositionRoundTrip[3];
  output Real eciVelocity[3];
  output Real ecefVelocityRoundTrip[3];
  output Real eciAcceleration[3];
  output Real ecefAccelerationRoundTrip[3];
  output Real blockECEF[3];
  output Real blockGeodetic[3];
  output Real blockNEDRoundTrip[3];
equation
  geodetic.latitude = latitude;
  geodetic.longitude = longitude;
  geodetic.altitude = altitude;
  ecef = ModelicaAerospace.Coordinates.geodeticToECEF(geodetic);
  geodeticRoundTrip = ModelicaAerospace.Coordinates.ecefToGeodetic(ecef);
  roundTrip = {
    geodeticRoundTrip.latitude,
    geodeticRoundTrip.longitude,
    geodeticRoundTrip.altitude};

  nedToECEF = ModelicaAerospace.Coordinates.nedToECEFMatrix(latitude, longitude);
  ecefToNED = ModelicaAerospace.Coordinates.ecefToNEDMatrix(latitude, longitude);
  nedVector = {120, -35, 8};
  ecefVector = ModelicaAerospace.Coordinates.nedToECEFVector(
    nedVector,
    latitude,
    longitude);
  nedRoundTrip = ModelicaAerospace.Coordinates.ecefToNEDVector(
    ecefVector,
    latitude,
    longitude);

  localGeodetic = ModelicaAerospace.Coordinates.nedToGeodetic(
    {100, 20, -5},
    geodetic);
  localNEDRoundTrip = ModelicaAerospace.Coordinates.geodeticToNED(
    localGeodetic,
    geodetic);

  eciPosition = ModelicaAerospace.Coordinates.ecefToECIVector(ecef, testTime);
  ecefPositionRoundTrip = ModelicaAerospace.Coordinates.eciToECEFVector(
    eciPosition,
    testTime);
  eciVelocity = ModelicaAerospace.Coordinates.ecefToECIVelocity(
    ecef,
    {120, -45, 8},
    testTime);
  ecefVelocityRoundTrip = ModelicaAerospace.Coordinates.eciToECEFVelocity(
    eciPosition,
    eciVelocity,
    testTime);
  eciAcceleration = ModelicaAerospace.Coordinates.ecefToECIAcceleration(
    ecef,
    {120, -45, 8},
    {1.2, -0.5, 0.25},
    testTime);
  ecefAccelerationRoundTrip =
    ModelicaAerospace.Coordinates.eciToECEFAcceleration(
      eciPosition,
      eciVelocity,
      eciAcceleration,
      testTime);

  geodeticBlock.latitude = latitude;
  geodeticBlock.longitude = longitude;
  geodeticBlock.altitude = altitude;
  blockECEF = geodeticBlock.ecef;
  ecefBlock.ecef = ecef;
  blockGeodetic = {ecefBlock.latitude, ecefBlock.longitude, ecefBlock.altitude};
  nedBlock.ned = nedVector;
  nedBlock.latitude = latitude;
  nedBlock.longitude = longitude;
  ecefVectorBlock.ecef = nedBlock.ecef;
  ecefVectorBlock.latitude = latitude;
  ecefVectorBlock.longitude = longitude;
  blockNEDRoundTrip = ecefVectorBlock.ned;
end CoordinateValidation;
