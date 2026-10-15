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
`cargo test` compares against golden outputs in `tests/golden/` and does NOT need the original DATCOM.
Maintainers regenerate goldens from the original (`gfortran datcom.f -o datcom`) with
`RUDATCOM_REFERENCE_BIN=/path/to/datcom` (script added with the test harness).
Naming: `rudatcom` is this project; bare `datcom` always means the original program.
