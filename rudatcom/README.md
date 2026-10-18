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

### Numerical foundation
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

### Geometry
`rudatcom_core::geometry::Geometry::from_case(&case)` computes reference quantities,
wing/tail/fin planforms (SETUP1/WTGEOM/ARCLSS), synthesis dimensions and hinge
rotation (SYNDIM/INFTGM), and body station resampling, maxima and integrals
(EQSPCE/GETMAX and BODYJM's geometric prefix). Named accessors accompany the
original 195-slot surface coefficient layout for downstream aerodynamic ports.
CONV normalizes an owned input copy to feet, applies model scaling and translates
the body nose; use parsed cases as inputs, not already converted copies.
Missing wing-derived reference quantities are `None`; invalid or degenerate
geometry produces an explicit error. Original flap-span corrections are retained
as warnings on the converted case.

Live geometry tests use all 11 sample decks plus `geometry/edges.inp` (all four
unit systems, scaling, nonzero nose offsets, negative sweeps, multi-panel wings,
single fins, hinge rotation and LARWB reference overrides). A temporary copy of
the original source receives control-flow hooks to snapshot COMMON blocks after
CONV and M02O02 and to stop BODYJM before its aerodynamic calculations; the
original calculation statements and installed source/binary are not changed.
These tests use the same compiler/source prerequisites and numeric tolerance as
the numerical probes, and leave no reference outputs in the repository.

### Airfoil sections
`rudatcom_core::airfoil` ports M50O62: NACA card decoding (4/5-digit, modified,
1-series, 6-series and supersonic S-1/S-2/S-3), user coordinates (`TYPEIN` 1/2),
Weber thickness/camber terms, lift-curve slope, CLMAX and the wing CSLOPE
slopes. `Geometry::from_case` runs it after CONV, before WTGEOM, filling only
the `xxSCHR` section inputs the user left unset. As in the original, a missing
speed reference (FLTCON) is an error, and an HT/VT/VF without `TYPE` is not
treated as straight tapered (the original reads outside its type-name table).
Inputs on which the original hangs or divides by zero produce an explicit error.

The live geometry test also compares every filled section slot, using all 11
samples plus `airfoil/sections.inp` (every NACA family, `TYPEIN` input, VF
cards, cranked/double-delta wings, high/low tails and a saved case).

### Flight conditions and subsonic body
`rudatcom_core::flight` ports ATMOS (the 1962 US Standard Atmosphere) and the
main program's LOOP 1/2/3 Mach/altitude sweep. It produces one `FlightPoint`
per analysed condition, with velocity, Reynolds number and speed regime.
`rudatcom_core::aero::analyze` then computes subsonic body-alone coefficients
(M04O04/M06O06): BODYRT slender-body plus crossflow, BODYJM (`METHOD=2`), and
BODOPT for cambered/asymmetric bodies, followed by the alpha derivatives.
`$EXPRnn` experimental body data replaces computed values, as in the original.
The original resets its body arrays at the start of every case, and so does
rudatcom.
Transonic, supersonic and hypersonic body points are not computed yet
(`body` is `None`).

`tests/aero.rs` compares every subsonic body point live against the original,
which is instrumented after M04O04/M06O06. It runs all 11 samples plus the decks in `tests/cases/body/`:
boat tails, Jorgensen with elliptic sections, rough/blunt bodies, asymmetric
bodies, LOOP/altitude/VINF sweeps, and experimental data carried by `SAVE`.
