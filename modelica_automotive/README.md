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
