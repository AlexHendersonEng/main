# Modelica Maritime

`ModelicaMaritime` is an open Modelica library for composing surface-vessel
and underwater-vehicle simulations from reusable, signal-oriented blocks. The
library targets OpenModelica and Rumoca and depends only on the Modelica
Standard Library.

The initial scaffold establishes package, interface, validation, and tooling
contracts. Dynamics, environment, propulsion, sensing, and guidance models are
added incrementally at reviewed checkpoints.

## Installation

The library requires Modelica Standard Library 4.0.0 and targets OpenModelica
and Rumoca. The Python validation environment is a standalone uv project:

```powershell
Set-Location modelica_maritime
uv sync --locked
uv run polaris backends
```

To expose the package through `MODELICAPATH`, add the directory containing
`ModelicaMaritime`:

```powershell
$env:MODELICAPATH = "$(Resolve-Path .);$env:MODELICAPATH"
omc
```

## Package layout

The library is rooted at `ModelicaMaritime\package.mo`. Its public namespaces
cover types, interfaces, constants, mathematics, coordinates, environment,
hydrostatics, hydrodynamics, vessel dynamics, propulsion, actuators, sensors,
guidance, navigation, control, utilities, examples, and packaged tests.

## Public conventions

- Public quantities use SI units. Angles are radians.
- Body axes are right-handed: x forward, y starboard, z down.
- Local navigation axes are north, east, down (NED).
- Quaternions are scalar-first `{w, x, y, z}` active rotations from body axes
  to the named reference frame.
- Generalized body velocity is `nu = {u, v, w, p, q, r}`. Translational
  components are in `m/s`; rotational components are in `rad/s`.
- Generalized body load is `tau = {X, Y, Z, K, M, N}`. Force components are
  in `N`; moment components are in `N.m`.
- Planar state is `{north, east, heading}` and planar body velocity is
  `{u, v, r}`.
- Depth and body z are positive down. Altitude above a seafloor is positive
  upward from the seafloor to the vehicle.
- Ground-relative velocity is relative to NED. Water-relative velocity is
  vehicle velocity minus the current velocity expressed in the same frame.
- Signal connectors are library-owned aliases of built-in Modelica types to
  preserve portability while keeping MSL as the only declared dependency.

## Mathematics and coordinates

`ModelicaMaritime.Mathematics` provides the marine-specific cross-product
matrix and heading wrapping. Heading wrapping uses the trigonometric principal
angle because Rumoca does not currently resolve the equivalent MSL helper.

`ModelicaMaritime.Coordinates` provides active 3-2-1 body-to-NED rotations,
body/NED vector transformations, planar surge/sway/yaw kinematics, and 6-DoF
Euler-angle kinematics. The Euler-rate transformation explicitly rejects
pitch at plus or minus 90 degrees; quaternion-based vehicle dynamics will be
used where global attitude coverage is required. Signed depth is the NED down
coordinate, while altitude above seafloor is seafloor depth minus vehicle
depth.

## Basic ocean properties

`ModelicaMaritime.Environment.Water` provides constant-property and linear
temperature/salinity water columns. The linear density model is an engineering
approximation around configurable reference conditions, not a TEOS-10
implementation. Hydrostatic pressure requires non-negative depth and uses
`p = p_surface + rho*g*depth` for constant density; `LinearWater` analytically
integrates its linear density profile over depth.

`ModelicaMaritime.Environment.Current` provides deterministic steady currents
and linear depth profiles in NED coordinates. Horizontal current direction is
the direction toward which the water flows, measured clockwise from north;
vertical current is positive down.

## Generic planar vessel dynamics

`ModelicaMaritime.VesselDynamics.Generic.Planar3DOF` integrates surge, sway,
and yaw with a symmetric rigid-body matrix, a user-supplied symmetric added-
inertia matrix, rigid-body and added-mass Coriolis terms, linear and quadratic
water-relative damping, external body loads, and steady NED current input.
Added-mass parameters use positive inertia values rather than the negative
hydrodynamic-derivative sign convention.

The model exposes ground-relative body velocity, current velocity in body
axes, water-relative velocity, acceleration, kinetic energy, and dissipation
power. Current is assumed spatially uniform and slowly varying; current
acceleration and current vorticity are omitted from this first generic model.
The total inertia matrix is required to be symmetric positive definite.

`ModelicaMaritime.Hydrodynamics` supplies reusable planar rigid-body mass,
Coriolis, damping, and coefficient-scaling models. Coefficient loads use
`0.5*rho*U^2*L*T` for surge and sway and an additional reference length for
yaw moment. `TablePlanarCoefficients` wraps the MSL `CombiTable1Ds` object for
linear interpolation and configurable extrapolation. Rumoca 0.10 does not
lower that MSL native table constructor, so the dedicated interpolation test
is OpenModelica-only.

## Integrated examples

`ModelicaMaritime.Examples.SurfaceManeuvering` accelerates a generic vessel,
applies a finite yaw moment, and continues in steady current while exercising
added inertia, Coriolis terms, water-relative damping, and planar kinematics.

## MMG-style maneuvering

`ModelicaMaritime.VesselDynamics.MMG` separates hull, propeller, and rudder
loads so each component can be parameterized and validated independently.
`HullLoads` uses normalized sway velocity and yaw rate with configurable
low-order polynomial coefficients. `PropellerLoads` uses an open-water
quadratic thrust-coefficient polynomial with explicit nominal wake fraction
and thrust deduction. `RudderLoads` uses axial propeller inflow, local lateral
inflow, normal-force slope, steering-resistance deduction, and hull-rudder
interaction coefficients.

`MMGLoads` exposes every component load and their exact sum.
`MMG.Planar3DOF` composes those loads over the generic planar plant; its
inherited `generalizedForceBody` input represents additional external loads.
The supplied parameter records contain no hard-coded vessel data, allowing
published benchmark or user vessel coefficients to be supplied explicitly.

The implementation is a low-order MMG-style engineering abstraction. Wake
fraction is presently constant, propeller thrust uses a quadratic `KT(J)`
curve, rudder inflow uses configurable axial and lateral scale factors, and
shallow-water, bank, drift-dependent wake, and detailed propeller-rudder race
corrections are not yet included. Parameter assertions reject non-positive
geometry and invalid interaction fractions.

`ModelicaMaritime.Examples.MMGTurningCircle` demonstrates a constant-rudder
turn with synthetic validation coefficients. Packaged tests also cover
straight-ahead equilibrium, independently calculated component loads, and a
heading-triggered 10-degree/10-degree zig-zag maneuver.

## Validation

Fast compiler-free checks:

```powershell
Set-Location modelica_maritime
uv sync --locked
uv run ruff check scripts tests
uv run python scripts\check_package.py
uv run pytest -m "not integration"
```

Run integrations for installed compilers:

```powershell
uv run pytest -m integration
```

Set `MODELICA_MARITIME_BACKENDS` to require specific backends. A requested
backend that is not installed fails instead of skipping:

```powershell
$env:MODELICA_MARITIME_BACKENDS = "openmodelica,rumoca"
uv run pytest -m integration
```

`TRACEABILITY.md` maps every public executable class to its requirement,
reference basis, and direct validation.
