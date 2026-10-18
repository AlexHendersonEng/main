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

### Flight conditions and body alone
`rudatcom_core::flight` ports ATMOS (the 1962 US Standard Atmosphere) and the
main program's LOOP 1/2/3 Mach/altitude sweep. It produces one `FlightPoint`
per analysed condition, with velocity, Reynolds number and speed regime.
`rudatcom_core::aero::analyze` then computes body-alone coefficients in every
regime:
- **Subsonic** (M04O04/M06O06): BODYRT slender-body plus crossflow, BODYJM
  (`METHOD=2`) and BODOPT for cambered/asymmetric bodies, followed by the
  alpha derivatives.
- **Transonic** (MAIN03): BODYRT slopes and drag at the first alpha, and
  SYPBOD at Mach 1.4 for its work values. Later alphas are not computed. As
  in the original, the case's first transonic point evaluates the cutoff
  Reynolds number at B(1) = UNUSED, a side effect of SETUP2's pre-pass.
- **Supersonic** (M19O23): SYPBOD. With `HYPERS` and `METHOD=1` it is
  replaced by **hypersonic** HYPBOD (M26O32).

`$EXPRnn` experimental body data replaces computed values, as in the original.
The original resets its body arrays at the start of every case, and so does
rudatcom. There are three gaps:
- No body is computed when the original skips it: ground effect, power, or
  the LARWB/TRNJET/hypersonic-flap paths.
- A transonic point whose pre-pass involves wing or horizontal-tail analyses
  is marked `deferred` until those methods are ported.
- Without BLN, the original's transonic SYPBOD values are NaN and never
  used, so rudatcom omits them.

`tests/aero.rs` compares every body point live against the original,
which is instrumented after M04O04/M06O06, before MAIN03's M12O14 and at the
end of MAIN04. It runs all 11 samples plus the decks in `tests/cases/body/`:
boat tails, Jorgensen with elliptic sections, rough/blunt bodies, asymmetric
bodies, LOOP/altitude/VINF sweeps, experimental data carried by `SAVE`,
custom STMACH/TSMACH, cone/ogive noses and boat-tail/flare tails at
transonic and supersonic speeds, and hypersonic bodies.

`rudatcom_core::aero::wing` ports the subsonic wing-alone methods into
`PointAero::wing`:
- **Lift** (M15O17): CALCA0 zero-lift angle with twist and camber
  corrections, WTLIFT lift-curve slope and CLmax, and LIFTCF lift curves for
  straight, double-delta, cranked and curved planforms.
- **Drag** (M03O03): CDRAG skin friction, induced and FIG53A suction terms.
- **Moment** (M31O37): CMALPH/FWDXAC aerodynamic centre and pitching moment,
  then normal/axial force and the alpha slopes.
- **Lateral** (M29O35/M17O21): SUBLAT wing CYbeta, Cnbeta and Clbeta,
  including partial-span dihedral.
- **Supersonic** (MAIN04's M18O22/M27O33/M23O27): SUPDRG skin-friction and
  wave drag, SUPLNG normal force (subsonic/supersonic leading edges, shock
  detachment, FIG60B), pitching moment and FWDXAC centre, and SUPLAT wing
  lateral derivatives.

The Mach-zero CLMCH0 pre-pass runs once per case, and `$EXPRnn` wing data
replaces computed values, as in the original. Wing results cover the case's
leading run of subsonic points followed by supersonic points without a body
or power; other points are marked `wing_deferred`.
The wing-body overlay (M07O07), body terms in SUBLAT/M29O35/SUPLAT and
the DWASH downwash quantities are left to later checkpoints.

`tests/wing.rs` instruments the original after `M17O21` and MAIN04's `M23O27`
and compares the WING results and the A/B/C/D/STB/WINGIN (plus supersonic
SLG) work arrays for every recorded point. It runs the 7 samples with wings plus `tests/cases/wing/`: straight,
double-delta, cranked and curved planforms, twist, experimental data,
forward sweep, partial dihedral, wing-body and supersonic (round/sharp
edges, rectangular, delta, double-delta and cranked) decks.
