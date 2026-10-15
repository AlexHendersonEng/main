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
gfortran datcom.f -o datcom
export RUDATCOM_REFERENCE_BIN=/path/to/datcom
cargo test
```
Without `RUDATCOM_REFERENCE_BIN` the reference tests fail with a clear message. No reference
outputs are stored in the repo; only input decks (`tests/cases/`).
Naming: `rudatcom` is this project; bare `datcom` always means the original program.
