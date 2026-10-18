//! Live differential test of the ported aerodynamic methods. The original is
//! rebuilt with control-flow hooks only (no formulas changed) that record
//! its COMMON-block results; nothing from the original is stored.

mod support;

use rudatcom_core::{aero::analyze, geometry::Geometry, input::parse_deck};
use std::{collections::HashMap, fs};

/// Build the original with probe hooks: CASE markers at the end of CHECK,
/// a subsonic flag around MAIN00's CALL MAIN01, and a body snapshot after
/// MAIN01's M04O04/M06O06 choice.
fn reference() -> support::Reference {
    let source = support::original_source().replace('\r', "");
    let main01 = "        IF(LOASRT) GO TO 1030\n          CALL MAIN01\n";
    let overlay = "        IF(   FLAG   ) CALL M06O06\n";
    for hook in [support::CHECK_END, main01, overlay] {
        assert_eq!(source.matches(hook).count(), 1, "hook {hook:?}");
    }
    let source = source
        .replace(
            support::CHECK_END,
            &format!("{}\n      CALL RUDATCOM_CASE_PROBE", support::CHECK_END),
        )
        .replace(
            main01,
            "        IF(LOASRT) GO TO 1030\n          CALL RUDATCOM_SUBSONIC(.TRUE.)\n          CALL MAIN01\n          CALL RUDATCOM_SUBSONIC(.FALSE.)\n",
        )
        .replace(
            overlay,
            &format!("{overlay}        CALL RUDATCOM_BODY_PROBE\n"),
        );
    support::compile(&source, include_str!("body_driver.f90"))
}

/// Decks: the 11 samples plus body-specific edge cases.
fn decks() -> Vec<(String, String)> {
    let dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../tests/cases");
    let mut decks: Vec<_> = (1..=11)
        .map(|n| format!("ex{n}.inp"))
        .chain(
            ["methods", "asymmetric", "flight", "experimental"]
                .iter()
                .map(|n| format!("body/{n}.inp")),
        )
        .map(|name| {
            let text = fs::read_to_string(dir.join(&name)).unwrap();
            (name, text.replace('\r', ""))
        })
        .collect();
    decks.sort_by_key(|d| d.0.starts_with("body/"));
    decks
}

/// One recorded original MAIN01 body snapshot: row name -> values.
type Snapshot<'a> = HashMap<&'a str, Vec<f32>>;

fn snapshots(case: &str) -> Vec<Snapshot<'_>> {
    case.split("POINT")
        .skip(1)
        .map(|point| {
            let mut rows = HashMap::new();
            for (n, line) in point.lines().enumerate() {
                let mut words = line.split_whitespace();
                let name = if n == 0 {
                    "POINT"
                } else {
                    words.next().unwrap()
                };
                rows.insert(name, words.map(|s| s.parse().unwrap()).collect());
            }
            rows
        })
        .collect()
}

#[test]
fn subsonic_body_matches_original() {
    let reference = reference();
    let mut compared = 0;
    for (deck, input) in decks() {
        let parsed = parse_deck(&input).unwrap();
        let text = support::run_original(&reference, &input, "body.txt");
        let cases: Vec<_> = text.split("CASE\n").skip(1).collect();
        assert_eq!(cases.len(), parsed.cases.len(), "{deck}: case count");
        for (index, (case, recorded)) in parsed.cases.iter().zip(cases).enumerate() {
            let geometry = Geometry::from_case(case).unwrap();
            let aero = analyze(&geometry).unwrap_or_else(|e| panic!("{deck} case {index}: {e}"));
            let original = snapshots(recorded);
            let rust: Vec<_> = aero.points.iter().filter(|p| p.body.is_some()).collect();
            assert_eq!(original.len(), rust.len(), "{deck} case {index}: points");
            for (point, (o, r)) in original.iter().zip(rust).enumerate() {
                let at =
                    |name: &str, i: usize| format!("{deck} case {index} point {point} {name}({i})");
                let check = |name: &str, i: usize, original: f32, rust: f32| {
                    support::assert_close(original, rust, &at(name, i));
                };
                let f = &r.flight;
                check("MACH", 0, o["POINT"][0], f.mach);
                check("RN", 0, o["POINT"][1], f.reynolds);
                let b = r.body.as_ref().unwrap();
                let coef = &o["COEF"];
                for (block, values) in [
                    &b.cd, &b.cl, &b.cm, &b.cn, &b.ca, &b.cla, &b.cma, &b.cyb, &b.cnb, &b.clb,
                ]
                .iter()
                .enumerate()
                {
                    for (j, &v) in values.iter().enumerate() {
                        check("BODY", block * 20 + j + 1, coef[block * 20 + j], v);
                    }
                }
                let drag = [
                    b.length,
                    b.maximum_area,
                    b.base_area,
                    b.fineness_ratio,
                    b.length_reynolds,
                    b.cutoff_reynolds,
                    b.skin_friction,
                    b.wetted_area,
                    b.friction_drag,
                    b.base_drag,
                    b.zero_lift_drag,
                ];
                for (i, (&o, r)) in o["DRAG"].iter().zip(drag).enumerate() {
                    check("DRAG", i, o, r);
                }
                let m = &o["METHOD"];
                for (i, v) in [b.apparent_mass_factor, b.eta, b.alpha_zero, b.cm0]
                    .into_iter()
                    .enumerate()
                {
                    if let Some(v) = v {
                        check("METHOD", i, m[i], v);
                    }
                }
                for (j, &v) in b.crossflow_drag.iter().enumerate() {
                    check("CDC", j, m[4 + j], v);
                }
                compared += 1;
            }
        }
    }
    assert!(compared > 0, "no body points compared");
}
