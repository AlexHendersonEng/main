# Modelica Aerospace

`ModelicaAerospace` is an open Modelica library for composing aerospace
simulations from reusable, signal-oriented blocks. The library targets
OpenModelica and Rumoca and depends only on the Modelica Standard Library.

This checkpoint establishes the package hierarchy and compiler compatibility
harness. Domain models and stable public interfaces will be added in later,
reviewed checkpoints.

## Python environment

The structural checks and integration tests are managed as a standalone uv
project. The environment installs `polaris` from a pinned commit of this
repository so later Polaris changes cannot silently alter the validation
environment:

```powershell
Set-Location modelica_aerospace
uv sync
```

Update the pinned commit deliberately in `pyproject.toml`, then run `uv lock`
and the full validation suite before accepting a newer Polaris revision.

## Package layout

The Modelica package is located at `ModelicaAerospace\package.mo`. Its public
namespaces cover types, interfaces, mathematics, coordinates, environment
models, flight dynamics, vehicle subsystems, guidance/navigation/control,
utilities, and examples.

## Public conventions

- Public quantities use SI units. Angles are radians; degree display units are
  metadata only.
- Body axes are right-handed: x forward, y starboard, z down.
- Local navigation axes are north, east, down (NED).
- ECEF uses x through the equator and prime meridian, y through 90 degrees east,
  and z through the north pole. ECI is aligned with ECEF at the model epoch.
- Quaternions are scalar-first `{w, x, y, z}` and represent active rotations
  from body coordinates to the named reference frame.
- Body angular velocity is ordered `{p, q, r}` about body `{x, y, z}`.
- Aerodynamic body-force coefficients are `{CX, CY, CZ}` and moment
  coefficients are `{Cl, Cm, Cn}`.
- Signal connectors are library-owned aliases of built-in Modelica types. This
  avoids requiring compiler support for MSL connector classes while preserving
  MSL as the library's only declared dependency.

## Attitude mathematics

`ModelicaAerospace.Mathematics` provides pure functions for vector normalization,
cross products, skew matrices, quaternion products and kinematics, 3-2-1 Euler
conversions, direction-cosine matrices, rotation validation, and active vector
rotation. `ModelicaAerospace.Mathematics.Blocks` exposes the main attitude conversions
through signal-oriented blocks. Quaternion-to-DCM-to-quaternion tests compare
orientation using the absolute quaternion dot product because `q` and `-q`
represent the same rotation.

The package uses `Mathematics` rather than `Math` because Rumoca fails to
resolve a nested user package named `Math`.

## Geodesy and coordinate frames

`ModelicaAerospace.Coordinates` implements WGS-84 geodetic/ECEF conversion,
local NED rotations and displacements, and ECEF/ECI transformations. Rotating
frame velocity and acceleration conversions include transport, Coriolis, and
centripetal terms. Pole behavior defines longitude as zero, longitude outputs
use the `[-pi, pi]` range, and the Earth center is rejected because geodetic
coordinates are undefined there.

Compiler-executed validation models are shipped under
`ModelicaAerospace.Tests`, grouped into `Common`, `Mathematics`, and
`Coordinates` subpackages. Python tests run those package models and compare
their outputs with independent numerical references.

## Environment models

`ModelicaAerospace.Environment.Atmosphere` implements the U.S. Standard
Atmosphere 1976 from -5 km through 84.852 km geopotential altitude. Public
altitude inputs are geometric altitude. The atmosphere state includes
temperature, pressure, density, speed of sound, and Sutherland dynamic
viscosity; companion blocks calculate Mach number and dynamic pressure.

`ModelicaAerospace.Environment.Gravity` provides constant NED gravity,
spherical inverse-square gravity, WGS-84 normal gravity with altitude
correction, Earth-rotation transport velocity, and centrifugal acceleration.
`ModelicaAerospace.Environment.Wind` provides steady NED wind, linear shear,
one-minus-cosine gusts, and seeded low-altitude Dryden filters. The Dryden
parameterization follows the MIL-F-8785C low-altitude formulas over 0 to
304.8 m and uses deterministic broadband forcing so compiler runs are
repeatable.

Compiler-executed environment validation models are shipped under
`ModelicaAerospace.Tests.Environment`; Python checks standard-atmosphere layer
boundaries, gravity reference points, wind/gust behavior, seeded trace
repeatability, RMS bounds and high-frequency attenuation.

## Flight dynamics

`ModelicaAerospace.FlightDynamics.PointMass` provides Cartesian NED and
flight-path-coordinate 3-DoF plants. Flight-path angle is positive upward and
ground track is clockwise from north; velocity-axis forces are ordered
`{tangential, upward-normal, right-lateral}`.

`ModelicaAerospace.FlightDynamics.RigidBody.FlatEarth` integrates body-axis
translation, Euler rigid-body rotation, and a scalar-first body-to-NED
quaternion. `SphericalEarth` integrates ECEF position and velocity with central
gravity, Coriolis and centrifugal acceleration, plus body-to-ECEF attitude.
Applied forces and moments use the documented body axes. Both plants expose
airspeed, angle of attack, sideslip, and quaternion norm; the flat model also
exposes Euler angles, ground track, flight-path angle, and load factor, while
the spherical model exposes geodetic position and orbital energy/momentum
diagnostics.

Initialization uses `PointMassInitialState`, `RigidBodyInitialState`, and
`SphericalInitialState` records. Packaged scenarios under
`ModelicaAerospace.Tests.FlightDynamics` cover equilibrium, analytic
constant-force and constant-moment motion, ballistic invariants, quaternion
norm preservation, and the short-time flat/spherical limiting case.

## Validation

Run structural tests:

```powershell
Set-Location modelica_aerospace
uv run pytest tests\test_package_structure.py
```

Run the real-compiler package smoke tests:

```powershell
Set-Location modelica_aerospace
uv run pytest -m integration tests\test_compiler_smoke.py
```

The integration tests skip an individual backend only when its compiler is not
installed.

Run the Python quality checks with:

```powershell
uv run ruff check scripts tests
uv run python scripts\check_package.py
```
