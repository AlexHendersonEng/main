# Modelica Maritime

`ModelicaMaritime` is an open Modelica library for composing surface-vessel
and underwater-vehicle simulations from reusable, signal-oriented blocks. The
library targets OpenModelica and Rumoca and depends only on the Modelica
Standard Library.

The library includes generic coefficient-based, MMG-style, and Fossen-style
dynamics together with environment, propulsion, actuator, sensing,
navigation, guidance, control, validation, and integrated example packages.

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

## Expanded maritime environment

`ProfileWater` and `TabulatedCurrent` provide continuous piecewise-linear
temperature, salinity, density, and NED-current profiles with endpoint
clamping. Hydrostatic pressure integrates the piecewise-linear density profile
exactly. The profile grid must contain at least two strictly increasing
depths. MSL table objects remain preferred for general interpolation, but
Rumoca 0.10 cannot lower their native table constructor; the small scalar
clamped-ramp interpolation used here is the portable compatibility exception.

`Environment.Wind` supplies steady and sinusoidally gusting deterministic wind
in NED coordinates. Wind direction is the direction toward which the air
moves, clockwise from north, and vertical velocity is positive down.

`Environment.Waves` supplies deep-water Airy regular-wave kinematics and a
finite-component deterministic irregular-wave realization. Spectrum choices
are Pierson-Moskowitz from wind speed, JONSWAP from significant height, peak
period, and peak enhancement, and Bretschneider from significant height and
peak period. Each component uses `a_i = sqrt(2*S(f_i)*delta_f)`; the exposed
zeroth moment is `sum(S(f_i)*delta_f)` and the realized significant height is
`4*sqrt(m0)`. Directions can vary per component, and default phases are
repeatable for a configurable integer seed. These are first-order deep-water
models: finite-depth dispersion, breaking, second-order drift, diffraction,
radiation memory, and vessel-generated waves are outside their validity.

`Environment.Bathymetry` provides flat and planar-sloped seafloors with NED
positive-down depth and positive altitude above the seafloor.
`Environment.Loads.QuadraticMediumLoad` applies drag opposite explicitly
supplied vehicle-minus-current or vehicle-minus-wind body velocity.
`LinearWaveLoad` maps wave elevation and body-frame particle velocity to a
first-order generalized load using user-supplied coefficients. Environment
generation is intentionally separate from load application.

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

All examples have finite experiment annotations and direct regression tests.

| Example | Capability exercised | Regression basis |
| --- | --- | --- |
| `PackageSmoke` | Independent package load and simulation | Constant analytic output on each requested compiler |
| `SurfaceManeuvering` | Generic 3-DoF acceleration, turning, and current response | Bounded pose, velocity, yaw rate, and non-negative dissipation |
| `MMGTurningCircle` | MMG hull/propeller/rudder turning maneuver | Heading excursion, horizontal extent, and bounded velocities |
| `UnderwaterFreeDecay` | Fossen hydrostatic roll/pitch and relative-speed decay | Quaternion bound, decaying attitude/speed, and dissipative power |
| `SurfaceWaveResponse` | Fossen 6-DoF regular-wave excitation | Wave amplitude, forward progress, quaternion bound, and bounded motion |
| `UnderwaterDepthHeadingHold` | Closed-loop depth, heading, and surge hold | Final tracking errors and bounded translational/rotational rates |
| `UnderwaterWaypointBathymetry` | Waypoint following at fixed altitude over slope | Waypoint distance, altitude/depth agreement, and quaternion bound |
| `PropulsionFailureResponse` | Twin-thruster path response after one failure | Failure activation, forward progress, and bounded degraded motion |

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

## Fossen-style six-degree-of-freedom dynamics

`ModelicaMaritime.VesselDynamics.Fossen.SixDOF` implements the matrix equation

`M*der(nu) + C_RB(nu)*nu + C_A(nu_r)*nu_r + D(nu_r)*nu_r + g(eta) = tau`

using body velocity `nu = {u, v, w, p, q, r}` and a normalized scalar-first
body-to-NED quaternion. The Modelica equation system solves the mass-matrix
equation directly; the library does not recreate a general linear solver.
Built-in array operations and `cross` are used for matrix and moment algebra.
The implementation exposes damping and restoring loads as `-D*nu_r` and
`-g(eta)` on the right-hand side.

Rigid-body mass is assembled from mass, center of gravity, and inertia about
the center of gravity. User-supplied added mass uses positive inertia values.
Rigid-body Coriolis terms use ground-relative velocity; added-mass Coriolis
and damping use water-relative velocity. Current acceleration and vorticity
are omitted. Total inertia must be symmetric, positive, and strictly
diagonally dominant; this portable sufficient condition is intentionally
stricter than general positive definiteness.

Hydrostatic load combines weight and fixed-displacement buoyancy at separate
centers of gravity and buoyancy. It supports neutral, positive, and negative
buoyancy and low-order roll/pitch restoring behavior. Displaced volume and
buoyancy center are constant, so waterplane-area variation, emergence,
flooding, and nonlinear surface-piercing hydrostatics are outside this model.

MSL quaternion normalization and MultiBody frame utilities were evaluated for
this implementation. OpenModelica supports them, but Rumoca 0.10 cannot
resolve those nested MSL APIs. The small scalar-first quaternion conversion
and derivative functions are therefore retained as portable equivalents; MSL
functionality remains preferred wherever both target compilers can lower it.

Packaged tests cover rigid-body matrix mechanics, Coriolis power neutrality,
constant force and principal moment motion, neutral trim, positive and
negative buoyancy, invalid inertia rejection, hydrostatic free decay, frame
rotation, and quaternion normalization. `UnderwaterFreeDecay` is the
integrated example.

## Propulsion, control surfaces, ballast, and actuators

`Propulsion.OpenWaterPropeller` implements signed low-order open-water thrust
and torque using quadratic `KT(J)` and `KQ(J)` curves, wake fraction, thrust
deduction, and signed shaft rate. It exposes advance ratio, thrust, torque, and
shaft power. `FirstOrderShaft` provides reversible motor/shaft lag with
enable/failure shutdown. `FixedThruster` and `AzimuthThruster` map bounded
forward/reverse commands into six-component body loads at configurable
application points, so the same load can drive generic planar or Fossen
six-degree-of-freedom vehicles.

`Actuators.ControlSurface` is a low-order rudder, hydroplane, or fin model. It
uses axial relative water speed, configurable normal direction, lift slope,
profile drag, and application-point moments. It is intended for controls and
system studies, not stall, cavitation, ventilation, or detailed propeller-race
prediction.

`Actuators.Servo` provides bias, deadband, position and asymmetric slew-rate
limits, and a commanded failure position. MSL `Limiter` and
`SlewRateLimiter` blocks were evaluated and work in OpenModelica, but Rumoca
0.10 cannot resolve `Modelica.Blocks.Nonlinear`; `CommandLimiter` therefore
retains the equivalent portable state equations as a compiler-compatibility
exception.

`BallastTank` integrates bounded ballast-water mass and exposes its positive-
down weight load. `VariableBuoyancy` integrates displaced volume and exposes
the incremental upward buoyancy load. These abstractions do not automatically
modify vehicle inertia or center-of-gravity/buoyancy records; users must
couple those effects separately when required. `EnergyStore` integrates
bounded charge/discharge energy, enforces power and empty/full limits, and
exposes state of charge. Detailed electrical networks remain outside the
initial library.

## Sensors, navigation, guidance, and control

`Sensors` provides ideal scalar/vector pass-through sensors and deterministic
non-ideal primitives with configurable bias, bounded repeatable forcing, and
quantization. Maritime assemblies cover NED position/ground velocity, wrapped
heading, water-relative speed/log, acceleration and angular rate, pressure-
derived positive-down depth, positive-up seafloor altitude, and horizontal
range/bearing. The deterministic forcing is intended for repeatable system
tests; it is not a stochastic sensor-error certification model.

`Navigation` provides ground speed/track, depth rate, and sideslip outputs;
NED/yaw dead reckoning; scalar complementary filtering with shortest-angle
correction; and complementary position propagation/correction. These are
basic low-order navigation building blocks, not a covariance-based INS,
Kalman filter, SLAM, or geodetic navigation system.

`Guidance` provides horizontal line-of-sight tracking, three-dimensional NED
waypoints, depth/altitude command selection, current-compensated heading, and
wrapped heading/depth/speed errors. Depth remains positive down, altitude is
positive upward from the seafloor, and current compensation subtracts current
from desired ground velocity to obtain the required through-water heading and
speed.

`Control.LimitedPID` implements proportional, integral, optional filtered
derivative, output saturation, and back-calculation anti-windup.
`HeadingController` applies shortest-angle heading error, while
`PlanarAllocator` maps normalized surge/yaw demand to port/starboard thrusters
and rudder. MSL `LimPID` was evaluated first and works in OpenModelica, but
Rumoca 0.10 cannot resolve `Modelica.Blocks.Continuous` or its controller
enumerations; the portable controller equations are retained for strict
dual-backend support.

Packaged closed-loop tests cover a twin-thruster surface vessel following an
LOS path in cross-current and a Fossen six-degree-of-freedom underwater
vehicle tracking heading, depth, speed, and a three-dimensional waypoint.
The finite underwater mission latches waypoint capture and removes commanded
yaw moment after entry so the vehicle settles rather than chasing an
ill-defined bearing at zero range.

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

The repository currently has no `.github\workflows` infrastructure, so this
package does not add an unverified compiler-install workflow. The commands
above are the authoritative local/CI gates; strict jobs should set
`MODELICA_MARITIME_BACKENDS` to the compiler assigned to that job.
