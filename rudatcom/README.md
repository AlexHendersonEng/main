# rudatcom

A Rust rewrite of the USAF Digital DATCOM (originally Fortran), aiming for near-identical output.

## Layout
- `crates/rudatcom-core` – library (parsing, methods, output)
- `crates/rudatcom-cli` – `rudatcom` command line tool
- `crates/rudatcom-py` – Python bindings (`import rudatcom`, built with maturin)

## Build
```sh
cargo build            # core + CLI
cargo test
cargo clippy --all-targets
cargo fmt --check
cd crates/rudatcom-py && maturin develop   # Python bindings
```

## Testing
Tests compare rudatcom against the ORIGINAL Digital DATCOM, which developers must build locally:
```sh
gfortran -std=legacy -ffree-form -fallow-argument-mismatch datcom.f -o datcom
export RUDATCOM_REFERENCE_BIN=/path/to/datcom
cargo test
```
Without `RUDATCOM_REFERENCE_BIN` the reference tests fail with a clear message. No reference
outputs are stored in the repo; only input decks (`tests/cases/`).
Naming: `rudatcom` is this project; bare `datcom` always means the original program.

### Numerical foundation (checkpoint 5)
`rudatcom-core::interp` ports GLOOK/SWITCH, TBFUNX, the TLIN family and INTERX;
`rudatcom-core::math` ports QUAD, QUADIN, TRAPZ, DET4, SIMUL4 and AGENR/GDELTA.
Their shared DATA tables are in `tables`; method-specific figure tables will be
ported with the aerodynamic methods that consume them.

Routine-level differential tests also require `gfortran` on PATH and the original
`datcom.f` beside `RUDATCOM_REFERENCE_BIN`. If the source is elsewhere, set
`RUDATCOM_REFERENCE_SOURCE=/path/to/datcom.f`. Tests extract unmodified routines,
compile a temporary probe with the legacy/free-form flags above, and compare
values live (relative tolerance `1e-4`, absolute floor `1e-6`); neither reference
outputs nor the original source are stored in this repository.

The numerical APIs retain `f32` evaluation order, rounded DATCOM constants and
GLOOK's knot snapping. TLIN uses X2-fastest flat storage; INTERX uses X1-fastest
storage and supports only one to three axes because the original's four-axis
dispatch is disabled (use TLIN4X through `tlin` for four axes). Extrapolation is
returned as metadata for later report formatting. Invalid dimensions, duplicate
knots, singular systems and non-finite results return errors rather than legacy
NaNs or undefined memory accesses.
GDELTA's safe API requires normalized, ordered flap spans (`0 <= EFI <= EFO <= 1`).
