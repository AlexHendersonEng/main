//! Live routine-level differential tests; no original outputs are stored.
//!
//! A probe links unmodified routines extracted from the developer's datcom.f.
//! It complements the whole-program tests before rudatcom can emit reports.

mod support;

use rudatcom_core::{
    interp::{self, Extrapolation, Limits},
    math,
};
use std::{
    fmt::Write,
    fs,
    process::{Command, Stdio},
};
fn reference() -> support::Reference {
    let routines = [
        "QUAD", "QUADIN", "TRAPZ", "DET4", "SIMUL4", "AGENR", "GDELTA", "GLOOK", "SWITCH",
        "TBFUNX", "TLIN1X", "TLINEX", "TLIN3X", "TLIN4X", "INTERX", "MESSGE",
    ];
    let source = support::original_source();
    support::compile(
        &support::extract(&source, &routines),
        include_str!("numerics_driver.f90"),
    )
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
    for (row, (actual, expected)) in actual.iter().zip(expected).enumerate() {
        assert_eq!(actual.len(), expected.len(), "row {row}");
        for (col, (&original, &rust)) in actual.iter().zip(expected).enumerate() {
            support::assert_close(original, rust, &format!("row {row}, col {col}"));
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

/// End-to-end parser -> CONV -> geometry comparison. Only control-flow hooks
/// change in the temporary Fortran source; no original formulas are replaced.
#[test]
fn sample_geometry_matches_original_common_blocks() {
    use rudatcom_core::{
        geometry::Geometry,
        input::{parse_deck, UNUSED},
    };
    let source = support::original_source().replace('\r', "");
    // Hook after CHECK's flight-condition fixes and ATMOS fill.
    let flc = support::CHECK_END;
    let body = "      XC = 2.0*XC/SP";
    assert_eq!(source.matches(flc).count(), 1);
    assert_eq!(source.matches(body).count(), 1);
    // CHECK then skips aerodynamic execution and proceeds to the next saved
    // input case; the probe itself runs M50O62 before the geometry.
    let source = source
        .replace(
            flc,
            &format!("{flc}\n      CALL RUDATCOM_GEOMETRY_PROBE\n      IERR=.TRUE.\n      RETURN"),
        )
        .replace(body, &format!("{body}\n      RETURN"));
    let reference = support::compile(&source, include_str!("geometry_driver.f90"));
    let dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../tests/cases");
    let mut decks: Vec<(String, String)> = (1..=11)
        .map(|sample| {
            (
                format!("ex{sample}"),
                fs::read_to_string(dir.join(format!("ex{sample}.inp"))).unwrap(),
            )
        })
        .collect();
    decks.push((
        "geometry-edges".into(),
        include_str!("../../../tests/cases/geometry/edges.inp").into(),
    ));
    // NACA families, TYPEIN coordinates, VF cards and saved-case sections.
    decks.push((
        "airfoil-sections".into(),
        include_str!("../../../tests/cases/airfoil/sections.inp").into(),
    ));
    for (sample, input) in decks {
        let input = input.replace('\r', "");
        let parsed = parse_deck(&input).unwrap();
        let text = support::run_original(&reference, &input, "geometry.txt");
        let cases: Vec<_> = text
            .split("CASE\n")
            .filter(|s| !s.trim().is_empty())
            .collect();
        assert_eq!(
            cases.len(),
            parsed.cases.len(),
            "ex{sample}: missing original cases"
        );
        for (index, (case, snapshot)) in parsed.cases.iter().zip(cases).enumerate() {
            let geometry = Geometry::from_case(case)
                .unwrap_or_else(|e| panic!("ex{sample} case {index}: {e}"));
            let rows: std::collections::HashMap<&str, Vec<f32>> = snapshot
                .lines()
                .map(|line| {
                    let mut words = line.split_whitespace();
                    let name = words.next().unwrap();
                    (name, words.map(|s| s.parse::<f32>().unwrap()).collect())
                })
                .collect();
            let check = |name: &str, values: &[f32]| {
                let original = rows
                    .get(name)
                    .unwrap_or_else(|| panic!("ex{sample}: missing {name}"));
                assert_eq!(original.len(), values.len(), "{name}");
                for (col, (&o, &r)) in original.iter().zip(values).enumerate() {
                    support::assert_close(
                        o,
                        r,
                        &format!("ex{sample}, case {index}, {name}({})", col + 1),
                    );
                }
            };
            let get = |name| geometry.inputs.case.namelist(name).unwrap();
            // INFTGM rotates XH/ZH after the CONV snapshot; compare with the
            // independent normalized input, before the hinge rotation.
            let converted = rudatcom_core::geometry::convert_case(case).unwrap();
            let syn0 = &converted.case.namelist("SYNTHS").unwrap().values;
            check(
                "SYN",
                &syn0[..9]
                    .iter()
                    .chain(&syn0[10..])
                    .copied()
                    .collect::<Vec<_>>(),
            );
            let f = get("FLTCON");
            check(
                "FLC",
                &["RNNUB", "ALT", "PINF", "TINF", "VINF", "WT"]
                    .iter()
                    .flat_map(|name| f.get(name).unwrap().iter().copied())
                    .collect::<Vec<_>>(),
            );
            check("TVT", &get("TVTPAN").values[154..162]);
            // M50O62 section characteristics, by xxIN slot (VFIN(k) is
            // VFSCHR location 162 + k in the shared /VTI/ block).
            let slots: Vec<usize> = std::iter::once(10)
                .chain(16..=69)
                .chain(72..=91)
                .chain([93])
                .collect();
            for (row, plnf, list, offset, cslope) in [
                ("WSCHR", "WGPLNF", "WGSCHR", 0, true),
                ("HSCHR", "HTPLNF", "HTSCHR", 0, false),
                ("VSCHR", "VTPLNF", "VTSCHR", 0, false),
                ("FSCHR", "VFPLNF", "VFSCHR", 162, false),
            ] {
                if !get(plnf).present {
                    assert!(!rows.contains_key(row), "missing Rust surface {row}");
                    continue;
                }
                let v = &get(list).values;
                let mut values: Vec<f32> = slots.iter().map(|s| v[s - 1 + offset]).collect();
                if cslope {
                    values.extend_from_slice(&v[94..100]);
                }
                check(row, &values);
            }
            let syn = &get("SYNTHS").values;
            check(
                "POSTSYN",
                &syn[..9]
                    .iter()
                    .chain(&syn[10..])
                    .copied()
                    .collect::<Vec<_>>(),
            );
            check("GROUND", get("GRNDEF").get("GRDHT").unwrap());
            for (group, row) in [
                ("PROPWR", "POWER"),
                ("JETPWR", "POWER"),
                ("LARWB", "POWER"),
                ("SYMFLP", "FLAP"),
                ("ASYFLP", "FLAP"),
                ("TRNJET", "FLAP"),
                ("HYPEFF", "FLAP"),
            ] {
                let n = get(group);
                if !n.present {
                    continue;
                }
                for var in n.def.vars.iter().filter(|v| !v.logical && v.loc > 0) {
                    for (i, &r) in n.get(var.name).unwrap().iter().enumerate() {
                        // Separate Rust namelists intentionally do not expose
                        // unrelated values left in overlaid Fortran COMMONs.
                        if r != UNUSED {
                            support::assert_close(
                                rows[row][var.loc - 1 + i],
                                r,
                                &format!("{sample}, case {index}, {group}.{}({i})", var.name),
                            );
                        }
                    }
                }
            }
            check(
                "REF",
                &[
                    geometry.reference.area.unwrap_or(UNUSED),
                    geometry.reference.chord.unwrap_or(UNUSED),
                    geometry.reference.roughness,
                    geometry.reference.span.unwrap_or(UNUSED),
                ],
            );
            for (name, surface) in [
                ("WG", &geometry.wing),
                ("HT", &geometry.horizontal_tail),
                ("VT", &geometry.vertical_tail),
                ("VF", &geometry.ventral_fin),
            ] {
                if let Some(s) = surface {
                    check(name, &s.coefficients);
                } else {
                    assert!(!rows.contains_key(name), "missing Rust surface {name}");
                }
            }
            if let Some(s) = geometry.synthesis {
                check(
                    "SYNGEOM",
                    &[
                        s.wing_body_le_offset,
                        s.cg_from_wing_body_le,
                        s.wing_height_intercept,
                        s.incidence_sine,
                        s.incidence_cosine,
                        s.incidence_tangent,
                        s.body_width_at_wing,
                    ],
                );
            }
            if let Some(b) = geometry.body {
                let bi = get("BODY");
                check(
                    "BODY",
                    &bi.values[..126]
                        .iter()
                        .chain(&bi.values[128..129])
                        .copied()
                        .collect::<Vec<_>>(),
                );
                let e = b.equally_spaced;
                check(
                    "EQUAL",
                    &e.stations
                        .x
                        .into_iter()
                        .chain(e.stations.radius)
                        .chain(e.stations.perimeter)
                        .chain(e.stations.area)
                        .chain(e.area_derivative)
                        .collect::<Vec<_>>(),
                );
                let mut stats = vec![
                    b.length,
                    b.base_area,
                    b.maximum_area,
                    b.maximum_area_station as f32,
                    b.volume,
                    b.projected_area,
                    b.projected_centroid,
                ];
                stats.extend(b.equivalent_radius);
                check("BODYSTATS", &stats);
            }
        }
    }
}
