//! Differential tests: rudatcom vs. the ORIGINAL Digital DATCOM, run live.
//!
//! Requires `RUDATCOM_REFERENCE_BIN` to point at the original binary
//! (`gfortran datcom.f -o datcom`). No reference outputs are stored in the repo.

use rudatcom_core::compare::{compare_reports, Tolerance};
use std::{
    fs,
    io::Write,
    path::{Path, PathBuf},
    process::{Command, Stdio},
};

/// Workspace-level `tests/` directory.
fn tests_dir() -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../../tests")
}

/// Path of the original DATCOM binary; fails the test with setup help if unavailable.
fn reference_bin() -> PathBuf {
    let value = std::env::var("RUDATCOM_REFERENCE_BIN").unwrap_or_else(|_| {
        panic!(
            "RUDATCOM_REFERENCE_BIN is not set. Build the original DATCOM \
             (gfortran datcom.f -o datcom) and set it to the binary's path."
        )
    });
    let path = PathBuf::from(value);
    assert!(
        path.is_file(),
        "RUDATCOM_REFERENCE_BIN does not point to a file: {path:?}"
    );
    path
}

/// Read an input deck, normalised to LF (the original fails on CRLF under Linux).
fn read_case(path: &Path) -> String {
    fs::read_to_string(path)
        .expect("cannot read case")
        .replace('\r', "")
}

/// Run the original DATCOM on `input` in a scratch directory and return its `datcom.out`.
fn run_reference(input: &str) -> String {
    let dir = tempfile::tempdir().expect("tempdir");
    fs::write(dir.path().join("case.inp"), input).unwrap();
    // The original prompts for the input file name on stdin and writes datcom.out to the cwd.
    let mut child = Command::new(reference_bin())
        .current_dir(dir.path())
        .stdin(Stdio::piped())
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .expect("cannot start the original DATCOM");
    child
        .stdin
        .take()
        .unwrap()
        .write_all(b"case.inp\n")
        .unwrap();
    assert!(
        child.wait().unwrap().success(),
        "original DATCOM exited with an error"
    );
    fs::read_to_string(dir.path().join("datcom.out")).expect("original wrote no datcom.out")
}

/// Names of all cases (`tests/cases/<name>.inp`), sorted.
fn case_names() -> Vec<String> {
    let mut names: Vec<String> = fs::read_dir(tests_dir().join("cases"))
        .expect("tests/cases missing")
        .filter_map(|e| {
            let p = e.ok()?.path();
            if p.extension()? != "inp" {
                return None;
            }
            Some(p.file_stem()?.to_string_lossy().into_owned())
        })
        .collect();
    names.sort();
    names
}

#[test]
fn reference_runs_every_case() {
    let names = case_names();
    assert_eq!(
        names.len(),
        11,
        "expected the 11 AFFDL-TR-79-3032 sample cases"
    );
    for name in names {
        let out = run_reference(&read_case(
            &tests_dir().join("cases").join(format!("{name}.inp")),
        ));
        assert!(
            !out.trim().is_empty(),
            "{name}: original produced empty output"
        );
    }
}

#[test]
fn reference_is_deterministic() {
    // Guards the comparison approach: two runs of the original must compare equal.
    let input = read_case(&tests_dir().join("cases/ex1.inp"));
    compare_reports(
        &run_reference(&input),
        &run_reference(&input),
        Tolerance::default(),
    )
    .expect("original output differs between runs");
}

/// Enabled once the port can produce full reports (Checkpoint 8).
#[test]
#[ignore = "output writer not ported yet"]
fn rudatcom_matches_reference() {
    for name in case_names() {
        let input = read_case(&tests_dir().join("cases").join(format!("{name}.inp")));
        let expected = run_reference(&input);
        let actual = rudatcom_core::run_case(&input).unwrap_or_else(|e| panic!("{name}: {e}"));
        compare_reports(&expected, &actual, Tolerance::default())
            .unwrap_or_else(|e| panic!("{name}: {e}"));
    }
}
