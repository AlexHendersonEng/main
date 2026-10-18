//! Shared live-Fortran probe build support; all generated sources/results are temporary.
// Each test binary uses a different subset of these helpers.
#![allow(dead_code)]

use rudatcom_core::compare::Tolerance;
use std::{fmt::Write, fs, path::PathBuf, process::Command};

pub struct Reference {
    pub dir: tempfile::TempDir,
    pub binary: PathBuf,
}

pub fn original_source() -> String {
    let binary = PathBuf::from(
        std::env::var_os("RUDATCOM_REFERENCE_BIN")
            .expect("set RUDATCOM_REFERENCE_BIN to the built original DATCOM; see README.md"),
    );
    assert!(
        binary.is_file(),
        "original DATCOM binary not found: {}",
        binary.display()
    );
    let source = std::env::var_os("RUDATCOM_REFERENCE_SOURCE")
        .map(PathBuf::from)
        .unwrap_or_else(|| binary.with_file_name("datcom.f"));
    fs::read_to_string(&source).unwrap_or_else(|e| {
        panic!(
            "cannot read {}: {e}; set RUDATCOM_REFERENCE_SOURCE to the original datcom.f",
            source.display()
        )
    })
}

pub fn extract(source: &str, routines: &[&str]) -> String {
    let mut extracted = String::new();
    for &name in routines {
        let mut found = false;
        let mut ended = false;
        for line in source.lines() {
            let s = line.trim_start().to_ascii_uppercase();
            if !found {
                if s.strip_prefix("SUBROUTINE ")
                    .is_some_and(|s| s.split(['(', ' ']).next() == Some(name))
                {
                    found = true;
                } else {
                    continue;
                }
            }
            writeln!(extracted, "{line}").unwrap();
            if s.trim() == "END" {
                ended = true;
                break;
            }
        }
        assert!(found && ended, "cannot extract {name} from original source");
    }
    extracted
}

pub fn compile(source: &str, driver: &str) -> Reference {
    let dir = tempfile::tempdir().unwrap();
    fs::write(dir.path().join("original.f"), source).unwrap();
    fs::write(dir.path().join("probe.f90"), driver).unwrap();
    let binary = dir.path().join(format!(
        "rudatcom-reference-probe{}",
        std::env::consts::EXE_SUFFIX
    ));
    let output = Command::new("gfortran")
        .args([
            "-std=legacy",
            "-fallow-argument-mismatch",
            "-ffree-form",
            "-ffree-line-length-none",
            "original.f",
            "probe.f90",
            "-o",
        ])
        .arg(&binary)
        .current_dir(dir.path())
        .output()
        .expect("gfortran is required for live numerical comparisons; see README.md");
    assert!(
        output.status.success(),
        "cannot build original numerical probe:\n{}",
        String::from_utf8_lossy(&output.stderr)
    );
    Reference { dir, binary }
}

/// End of CHECK's ATMOS fill (LF-normalized source): the point where the
/// original's flight conditions match rudatcom's converted FLTCON.
pub const CHECK_END: &str =
    "          CALL ATMOS(FLC(I+96), FLC(I+116), FLC(I+73))\n 1220   CONTINUE\n 1230 CONTINUE";

/// Run a (probe-instrumented) original on `input` in a scratch directory
/// inside the probe build and return the text of `result`.
pub fn run_original(reference: &Reference, input: &str, result: &str) -> String {
    use std::io::Write as _;
    use std::process::Stdio;
    let scratch = tempfile::tempdir_in(reference.dir.path()).unwrap();
    fs::write(scratch.path().join("case.inp"), input).unwrap();
    let mut child = Command::new(&reference.binary)
        .current_dir(scratch.path())
        .stdin(Stdio::piped())
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .unwrap();
    child
        .stdin
        .take()
        .unwrap()
        .write_all(b"case.inp\n")
        .unwrap();
    let output = child.wait_with_output().unwrap();
    assert!(
        output.status.success(),
        "original failed: {}",
        String::from_utf8_lossy(&output.stderr)
    );
    fs::read_to_string(scratch.path().join(result)).unwrap_or_default()
}

/// Whether two values agree within the default tolerance (both finite).
pub fn is_close(original: f32, rust: f32) -> bool {
    let tolerance = Tolerance::default();
    let error = (f64::from(original) - f64::from(rust)).abs();
    original.is_finite()
        && rust.is_finite()
        && error <= tolerance.abs + tolerance.rel * f64::from(original.abs().max(rust.abs()))
}

pub fn assert_close(original: f32, rust: f32, context: &str) {
    assert!(
        original.is_finite() && rust.is_finite(),
        "{context}: non-finite value"
    );
    let error = (f64::from(original) - f64::from(rust)).abs();
    assert!(
        is_close(original, rust),
        "{context}: original={original:e}, rudatcom={rust:e}, difference={error:e}"
    );
}
