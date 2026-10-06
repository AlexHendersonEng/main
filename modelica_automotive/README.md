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
- Roll is positive left-side-up and pitch is positive nose-up.
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
