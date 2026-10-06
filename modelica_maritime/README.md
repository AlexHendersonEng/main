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
