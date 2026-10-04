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
