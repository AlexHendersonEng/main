# Modelica Automotive

`ModelicaAutomotive` is an open Modelica library for composing automotive
vehicle-dynamics simulations from reusable, signal-oriented blocks. The
library targets OpenModelica and Rumoca and depends only on Modelica Standard
Library 4.0.0.

The library is under active development. The initial scaffold defines stable
physical conventions, common interfaces, validation tooling, and a compiler
smoke model; longitudinal, planar, tire, suspension, powertrain, control, and
scenario models will be added in reviewed phases.

## Installation

The Python validation environment is a standalone uv project:

```powershell
Set-Location modelica_automotive
uv sync --locked
uv run polaris backends
```

To expose the package through `MODELICAPATH`, add the directory containing
`ModelicaAutomotive`:

```powershell
$env:MODELICAPATH = "$(Resolve-Path .);$env:MODELICAPATH"
omc
```

## Public conventions

- Public quantities use SI units.
- Vehicle body axes are right-handed: x forward, y left, z upward.
- World axes are right-handed: X forward/reference-path north, Y left, Z
  upward.
- Yaw and steering angles are positive counter-clockwise when viewed from
  above.
- Roll is positive left-side-up and pitch is positive nose-down, following
  right-handed rotation about the positive body y-axis.
- Quaternions are scalar-first `{w, x, y, z}` active body-to-world rotations.
- Body angular velocity is ordered `{p, q, r}` about body `{x, y, z}`.
- Wheel-corner arrays are ordered `{frontLeft, frontRight, rearLeft,
  rearRight}`.
- Signal connectors are library-owned aliases of built-in Modelica types.

## Package layout

The public namespaces cover shared types and interfaces, mathematics, roads,
vehicle plants, tires, wheels, brakes, steering, suspension, aerodynamics,
powertrain, drivers, sensors, controls, scenarios, utilities, examples, and
packaged validation models.

## Longitudinal dynamics

`ModelicaAutomotive.VehicleDynamics.Longitudinal.Body` integrates forward
position and speed from an externally supplied net tire force. It includes
quadratic aerodynamic drag, smoothly regularized rolling resistance, and the
gravity component on a positive-uphill road grade. Force-producing tire and
powertrain models remain separate so they can be replaced independently.

`ModelicaAutomotive.Road` provides locally constant heading, grade, bank,
quadratic crown, elevation, and friction descriptions. `FourCornerRoad`
evaluates the same surface at wheel contact points ordered front-left,
front-right, rear-left, rear-right.

## Tires, wheels, and brakes

`ModelicaAutomotive.Tires` provides regularized wheel-slip kinematics and
linear, Fiala brush, and compact parameterized Magic Formula tire models.
Positive slip ratio produces forward force and positive slip angle produces
leftward force. Every model applies a combined friction-circle limit based on
normal load and road friction.

`ModelicaAutomotive.Wheels.RotationalDynamics` integrates wheel speed from hub,
signed brake, and tire reaction torques. Brake models include ideal commanded
torque, first-order actuation, clamp-force friction limiting, and an MSL
table-mapped option. The mapped brake is validated only with OpenModelica
because Rumoca 0.10 cannot lower the MSL native table constructor.

## Steering and planar handling

`ModelicaAutomotive.Steering` provides steering-wheel ratio and limits,
first-order rate-limited response, and Ackermann front-wheel geometry.
`VehicleDynamics.Planar` includes rear-axle kinematic bicycle, linear-tire
dynamic bicycle, and force-driven double-track plants.

The double-track plant transforms each wheel's longitudinal and lateral force
through its steering angle, integrates body translation and yaw, accepts
separate external body loads, and reports quasi-static longitudinal and
lateral corner-load transfer. Its load approximation is intended for
maneuvers where all four reported normal loads remain positive.

## Suspension and full-body dynamics

`ModelicaAutomotive.Suspension` provides corner spring-dampers, progressive
travel stops, anti-roll bars, unilateral vertical tire contact, a reduced
heave/roll/pitch body, and a four-corner sprung-body assembly. Positive corner
force is upward, positive roll raises the left side, and positive pitch lowers
the nose.

`VehicleDynamics.RigidBody.FullBody` integrates body-axis translation,
angular velocity, world position, and a scalar-first active body-to-world
quaternion. Applied body forces exclude gravity; the plant adds world-down
gravity internally. Suspension and tire forces remain external so full-body
plants can be composed with different corner models.

## Aerodynamics and powertrain

`ModelicaAutomotive.Aerodynamics.BodyLoads` converts relative body-axis air
velocity into drag, side force, lift, coefficient moments, and application-
point moments. `MappedCoefficients` provides an optional MSL table interface;
as with other MSL native table objects, its dedicated validation is
OpenModelica-only.

`ModelicaAutomotive.Powertrain` provides ideal and first-order torque sources,
speed-dependent drive limits, regenerative power flow, energy storage,
fixed/selectable gearing, open differentials, configurable front/rear torque
distribution, driveshaft compliance, and a regularized friction clutch.
Positive storage power discharges energy; negative storage power represents
charging. Transmission efficiencies are applied in the current direction of
power flow so losses remain non-negative.

## Drivers, sensors, and chassis controls

`ModelicaAutomotive.Drivers` provides bounded open-loop maneuver commands, a
PI speed controller with anti-windup and propulsion/brake splitting,
look-ahead path steering, and manual/automated command arbitration with
emergency-brake priority.

`ModelicaAutomotive.Sensors.IdealVehicleSensors` exposes ideal wheel, body,
steering, suspension, navigation, slip, and load signals.
`DeterministicSensor` adds configurable gain, bias, first-order lag,
deterministic sinusoidal error, and output limits without stochastic or
backend-dependent behavior.

`ModelicaAutomotive.Control` provides four-wheel anti-lock braking, driven-
wheel traction control, saturated direct-yaw-moment stabilization, and
regenerative/friction brake blending. All controllers use signal interfaces
so they can be connected to reduced-order or full-body plants.

## Validation

The fast required checks do not need a Modelica compiler:

```powershell
Set-Location modelica_automotive
uv sync --locked
uv run ruff check scripts tests
uv run python scripts\check_package.py
uv run pytest -m "not integration"
```

Run all available compiler integrations:

```powershell
uv run pytest -m integration
```

Set `MODELICA_AUTOMOTIVE_BACKENDS` for strict backend selection. A requested
compiler that is unavailable fails rather than skips:

```powershell
$env:MODELICA_AUTOMOTIVE_BACKENDS = "openmodelica,rumoca"
uv run pytest -m integration
```

`TRACEABILITY.md` maps every public executable class to its requirement,
implementation basis, and direct validation.
