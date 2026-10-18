//! Live routine-level differential tests; no original outputs are stored.
//!
//! A probe links unmodified routines extracted from the developer's datcom.f.
//! It complements the whole-program tests before rudatcom can emit reports.

use rudatcom_core::{
    compare::Tolerance,
    interp::{self, Extrapolation, Limits},
    math,
};
use std::{
    fmt::Write,
    fs,
    path::PathBuf,
    process::{Command, Stdio},
};

struct Reference {
    dir: tempfile::TempDir,
    binary: PathBuf,
}

fn reference() -> Reference {
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
    // The source contains Hollerith data: preserve significant blanks and
    // continuation markers rather than translating or regenerating routines.
    let bytes = fs::read(&source).unwrap_or_else(|e| {
        panic!(
            "cannot read {}: {e}; set RUDATCOM_REFERENCE_SOURCE to the original datcom.f",
            source.display()
        )
    });
    let source = String::from_utf8(bytes).expect("datcom.f must be UTF-8/ASCII");
    let routines = [
        "QUAD", "QUADIN", "TRAPZ", "DET4", "SIMUL4", "AGENR", "GDELTA", "GLOOK", "SWITCH",
        "TBFUNX", "TLIN1X", "TLINEX", "TLIN3X", "TLIN4X", "INTERX", "MESSGE",
    ];
    let mut extracted = String::new();
    for name in routines {
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
    let dir = tempfile::tempdir().unwrap();
    fs::write(dir.path().join("original.f"), extracted).unwrap();
    fs::write(
        dir.path().join("probe.f90"),
        include_str!("numerics_driver.f90"),
    )
    .unwrap();
    let binary = dir.path().join(format!(
        "rudatcom-numerics-probe{}",
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

/// File-backed stdin avoids a pipe deadlock when a batch contains many results.
fn compare(requests: &str, expected: &[Vec<f32>]) {
    let reference = reference();
    let run = tempfile::tempdir_in(reference.dir.path()).unwrap();
    let input = run.path().join("requests.txt");
    fs::write(&input, requests).unwrap();
    let output = Command::new(&reference.binary)
        .current_dir(run.path())
        .stdin(Stdio::from(fs::File::open(input).unwrap()))
        .output()
        .unwrap();
    assert!(
        output.status.success(),
        "original probe failed: {}",
        String::from_utf8_lossy(&output.stderr)
    );
    let stdout = String::from_utf8(output.stdout).unwrap();
    let actual: Vec<Vec<f32>> = stdout
        .lines()
        .map(|line| {
            line.split_whitespace()
                .map(|s| {
                    s.parse()
                        .unwrap_or_else(|_| panic!("unexpected original output: {line}"))
                })
                .collect()
        })
        .collect();
    assert_eq!(actual.len(), expected.len(), "original output:\n{stdout}");
    let tolerance = Tolerance::default();
    for (row, (actual, expected)) in actual.iter().zip(expected).enumerate() {
        assert_eq!(actual.len(), expected.len(), "row {row}");
        for (col, (&original, &rust)) in actual.iter().zip(expected).enumerate() {
            assert!(
                original.is_finite() && rust.is_finite(),
                "non-finite at {row}:{col}"
            );
            let error = (f64::from(original) - f64::from(rust)).abs();
            assert!(error <= tolerance.abs + tolerance.rel * f64::from(original.abs().max(rust.abs())),
                "row {row}, col {col}: original={original:e}, rudatcom={rust:e}, difference={error:e}");
        }
    }
}

fn numbers(requests: &mut String, values: &[f32]) {
    for value in values {
        write!(requests, "{value:e} ").unwrap();
    }
}

fn policy(code: i32) -> Extrapolation {
    match code {
        i if i < 0 => Extrapolation::HoldSilent,
        0 => Extrapolation::Hold,
        1 => Extrapolation::Linear,
        _ => Extrapolation::Quadratic,
    }
}

#[test]
fn quadratic_integration_and_solver_match_original() {
    let mut requests = String::new();
    let mut expected = Vec::new();
    let x = [-0.75, 0.2, 2.3];
    let y = [2.4, -0.3, 5.8];
    for at in [-2.0, -0.75, 0.0, 0.2, 1.0, 2.3, 4.0] {
        requests.push_str("1\n");
        numbers(&mut requests, &x);
        numbers(&mut requests, &y);
        writeln!(requests, "{at:e}").unwrap();
        let (v, d) = math::quad(&x, &y, at).unwrap();
        expected.push(vec![v, d]);
    }
    // All four remainder lengths, including multiple five-point panels.
    for n in 1..=16 {
        let samples: Vec<f32> = (0..n)
            .map(|i| (i as f32 * 0.7).sin() + 0.03 * (i * i) as f32)
            .collect();
        writeln!(requests, "2\n{n} 0.3").unwrap();
        numbers(&mut requests, &samples);
        requests.push('\n');
        expected.push(vec![math::quadin(&samples, 0.3).unwrap()]);
    }
    let x = [0.0, 0.1, 0.8, 1.1, 2.3, 4.0];
    let y = [0.3, 1.2, 0.7, 0.8, 1.5, 0.5];
    for mode in [-1, 0, 1] {
        writeln!(requests, "3\n6 {mode}").unwrap();
        numbers(&mut requests, &x);
        numbers(&mut requests, &y);
        requests.push('\n');
        expected.push(match mode {
            -1 => vec![math::frustum_volume(&x, &y).unwrap()],
            0 => math::trapz(&x, &y).unwrap(),
            _ => vec![*math::trapz(&x, &y).unwrap().last().unwrap()],
        });
    }
    for offset in [0.0, 0.3, 1.7] {
        let a = [
            3.0 + offset,
            0.2,
            -0.1,
            0.8,
            1.2,
            4.0,
            0.5,
            -0.4,
            0.0,
            0.7,
            2.8,
            0.3,
            -0.9,
            0.0,
            0.1,
            3.2,
        ];
        let rhs = [2.0, -3.0, 1.5, 0.7];
        requests.push_str("4\n");
        numbers(&mut requests, &a);
        numbers(&mut requests, &rhs);
        requests.push('\n');
        let mut result = vec![math::det4(&a).unwrap()];
        result.extend(math::simul4(&a, &rhs).unwrap());
        expected.push(result);
    }
    compare(&requests, &expected);
}

#[test]
fn lookup_and_curve_values_and_derivatives_match_original() {
    let mut requests = String::new();
    let mut expected = Vec::new();
    for axis in [vec![-1.0, 0.0, 0.8, 2.0], vec![2.0, 0.8, 0.0, -1.0]] {
        for at in [-2.0, -1.0005, -1.0, -0.3, 0.0, 0.7995, 0.8, 1.4, 2.0, 3.0] {
            writeln!(requests, "10\n{}", axis.len()).unwrap();
            numbers(&mut requests, &axis);
            writeln!(requests, "{at:e}").unwrap();
            let r = interp::glook(&axis, at).unwrap();
            expected.push(vec![r.index as f32, r.fraction.unwrap_or(-1.0)]);
        }
    }
    for n in 1..=5 {
        let axis = [-1.0, 0.0, 0.8, 2.0, 3.5];
        let values = [0.2, -0.5, 2.3, 1.7, 5.1];
        for lo in [-1, 0, 1, 2] {
            for hi in [0, 1, 2] {
                for at in [-2.0, -1.0, -0.4, 0.0, 0.3, 0.8, 1.4, 2.0, 3.5, 4.5] {
                    writeln!(requests, "6\n{n} {lo} {hi}").unwrap();
                    numbers(&mut requests, &axis[..n]);
                    numbers(&mut requests, &values[..n]);
                    writeln!(requests, "{at:e}").unwrap();
                    let r = interp::tbfunx(
                        &axis[..n],
                        &values[..n],
                        at,
                        Limits {
                            lower: policy(lo),
                            upper: policy(hi),
                        },
                    )
                    .unwrap();
                    expected.push(vec![r.value, r.derivative]);
                }
            }
        }
    }
    compare(&requests, &expected);
}

#[test]
fn multidimensional_tables_and_extrapolation_match_original() {
    let mut requests = String::new();
    let mut expected = Vec::new();
    // Unequal axis lengths and cross terms expose transposed table layouts.
    let x1 = [-0.5, 0.3, 1.7, 3.0];
    let x2 = [2.0, 0.8, -0.2];
    let x3 = [-1.0, 2.0, 4.0];
    let x4 = [0.0, 0.7, 2.0];
    let axes: [&[f32]; 4] = [&x1, &x2, &x3, &x4];
    for d in 1..=4 {
        let count: usize = axes[..d].iter().map(|x| x.len()).product();
        let values: Vec<f32> = (0..count)
            .map(|i| ((i * 17 % 31) as f32 * 0.4).cos() + i as f32 * 0.03)
            .collect();
        for mode in 0..=1 {
            if mode == 1 && d == 4 {
                continue;
            }
            for code in [-1, 0, 1, 2] {
                for query in 0..5 {
                    let at = match query {
                        0 => [0.3001, 1.3, 0.2, 1.0],
                        1 => [-1.0, 2.5, -2.0, -0.5],
                        2 => [3.5, -1.0, 5.0, 2.5],
                        3 => [0.3, 0.8, 2.0, 0.7],
                        _ => [0.0, 1.0, -1.0, 2.0],
                    };
                    writeln!(requests, "7\n{d} {mode}").unwrap();
                    for x in &axes[..d] {
                        write!(requests, "{} ", x.len()).unwrap();
                    }
                    for _ in 0..2 * d {
                        write!(requests, "{code} ").unwrap();
                    }
                    numbers(&mut requests, &at[..d]);
                    requests.push('\n');
                    for x in &axes[..d] {
                        numbers(&mut requests, x);
                        requests.push('\n');
                    }
                    numbers(&mut requests, &values);
                    requests.push('\n');
                    let limits = vec![
                        Limits {
                            lower: policy(code),
                            upper: policy(code)
                        };
                        d
                    ];
                    let r = if mode == 1 {
                        interp::interx(&axes[..d], &values, &at[..d], &limits)
                    } else {
                        interp::tlin(&axes[..d], &values, &at[..d], &limits)
                    }
                    .unwrap();
                    expected.push(vec![r.value]);
                }
            }
        }
    }
    compare(&requests, &expected);
}

#[test]
fn short_and_descending_axes_match_original() {
    let mut requests = String::new();
    let mut expected = Vec::new();
    // Singleton axes can hold endpoints; two-knot quadratic extrapolation
    // falls back to a line. Exercise independent lower/upper policies as well.
    for axis in [&[1.0][..], &[1.0, 0.2][..]] {
        for lo in [-1, 0, 1, 2] {
            for hi in [-1, 0, 1, 2] {
                if axis.len() == 1 && (lo > 0 || hi > 0) {
                    continue;
                }
                for at in [-0.5, 0.2, 0.7, 1.0, 2.0] {
                    let values = &[-0.3, 2.7][..axis.len()];
                    writeln!(requests, "7\n1 0\n{} {lo} {hi} {at:e}", axis.len()).unwrap();
                    numbers(&mut requests, axis);
                    requests.push('\n');
                    numbers(&mut requests, values);
                    requests.push('\n');
                    let r = interp::tlin(
                        &[axis],
                        values,
                        &[at],
                        &[Limits {
                            lower: policy(lo),
                            upper: policy(hi),
                        }],
                    )
                    .unwrap();
                    expected.push(vec![r.value]);
                }
            }
        }
    }
    compare(&requests, &expected);
}

#[test]
fn flap_loading_tables_and_both_branches_match_original() {
    let mut requests = String::new();
    let mut expected = Vec::new();
    for boch in [[2.0; 4], [1.3, 1.7, 2.1, 2.8], [0.5, 1.1, 3.0, 5.0]] {
        for sweep in [-10.0, 0.0, 25.0, 45.0] {
            requests.push_str("5\n");
            numbers(&mut requests, &boch);
            writeln!(requests, "{sweep:e}").unwrap();
            expected.push(math::agenr(&boch, sweep).unwrap().to_vec());
            for (efi, efo) in [(0.0, 1.0), (0.2, 0.75), (0.556, 0.831)] {
                requests.push_str("8\n");
                numbers(&mut requests, &boch);
                writeln!(requests, "{sweep:e} {efi:e} {efo:e}").unwrap();
                let r = math::gdelta(&boch, sweep, efi, efo).unwrap();
                expected.push(
                    r.full
                        .into_iter()
                        .chain(r.inboard)
                        .chain(r.outboard)
                        .collect(),
                );
            }
        }
    }
    for sweep in [0.0, 25.0, 45.0] {
        writeln!(requests, "9\n{sweep:e} 0.1 0.4 2.0 3.0").unwrap();
        expected.push(
            math::gdelta_asymmetric(sweep, 0.1, 0.4, 2.0, 3.0)
                .unwrap()
                .to_vec(),
        );
    }
    compare(&requests, &expected);
}

#[test]
fn degenerate_math_fails_instead_of_propagating_nans() {
    assert!(math::quad(&[1.0, 1.0, 2.0], &[0.0; 3], 0.0).is_err());
    assert!(math::simul4(&[0.0; 16], &[1.0; 4]).is_err());
    assert!(math::agenr(&[0.0; 4], 0.0).is_err());
    assert!(math::gdelta(&[2.0; 4], 0.0, 0.8, 0.2).is_err());
    assert!(math::gdelta(&[2.0; 4], 0.0, -0.1, 0.9).is_err());
    assert!(math::quadin(&[], 1.0).is_err());
    assert!(math::quadin(&[1.0], -1.0).is_err());
    assert!(math::trapz(&[1.0], &[]).is_err());
    assert!(math::trapz(&[0.0, 1.0], &[0.0, f32::NAN]).is_err());
}
