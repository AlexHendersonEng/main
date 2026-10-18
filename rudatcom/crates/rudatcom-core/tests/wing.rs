//! Live differential test of the wing-alone methods.
//! The original is rebuilt with control-flow hooks only (no formulas
//! changed) that record its wing COMMON blocks after MAIN01's and MAIN04's
//! lateral derivatives and before MAIN03's M49O61; nothing from the
//! original is stored.

mod support;

use rudatcom_core::{
    aero::{analyze, PointAero},
    flight::Regime,
    geometry::Geometry,
    input::{parse_deck, UNUSED},
};
use std::{collections::HashMap, fs};

/// Build the original with probe hooks: CASE markers at the end of CHECK, a
/// subsonic flag around MAIN00's CALL MAIN01, a wing snapshot after
/// MAIN01's M29O35/M17O21, a supersonic one after MAIN04's M23O27 and a
/// transonic one between MAIN03's M35O43 and M49O61.
fn reference() -> support::Reference {
    let source = support::original_source().replace('\r', "");
    let main01 = "        IF(LOASRT) GO TO 1030\n          CALL MAIN01\n";
    let lateral = "      CALL M29O35\n      CALL M17O21\n";
    let main04 = "      IF(NF .LT. 0) GO TO 1080\n      CALL M49O61\n";
    let main03 = "      CALL M35O43\n      CALL M49O61\n";
    for hook in [support::CHECK_END, main01, lateral, main04, main03] {
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
            lateral,
            &format!("{lateral}      IF(WGPL) CALL RUDATCOM_WING_PROBE\n"),
        )
        .replace(
            main04,
            &format!("      IF(NF .GE. 0 .AND. WGPL) CALL RUDATCOM_SUPERSONIC_PROBE\n{main04}"),
        )
        .replace(
            main03,
            "      CALL M35O43\n      IF(WGPL) CALL RUDATCOM_TRANSONIC_PROBE\n      CALL M49O61\n",
        );
    support::compile(&source, include_str!("wing_driver.f90"))
}

/// Decks: the samples with wings plus wing-specific edge cases.
fn decks() -> Vec<(String, String)> {
    let dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../tests/cases");
    (2..=8)
        .map(|n| format!("ex{n}.inp"))
        .chain(
            [
                "straight",
                "planforms",
                "twist",
                "experimental",
                "sweep",
                "dihedral",
                "body",
                "supersonic",
                "transonic",
            ]
            .iter()
            .map(|n| format!("wing/{n}.inp")),
        )
        .map(|name| {
            let text = fs::read_to_string(dir.join(&name)).unwrap();
            (name, text.replace('\r', ""))
        })
        .collect()
}

/// One recorded point: row name -> values.
fn snapshots(case: &str) -> Vec<HashMap<&str, Vec<f32>>> {
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

/// Points at which the original records a wing: subsonic MAIN01, MAIN03 and
/// MAIN04 calls (no deck here uses power, which skips MAIN04's methods).
fn recorded(p: &PointAero) -> bool {
    p.wing.is_some() || p.wing_deferred
}

#[test]
fn wing_matches_original() {
    let reference = reference();
    let mut compared = 0;
    let mut supersonic = 0;
    let mut transonic = 0;
    // Every mismatch is collected so one run shows the whole picture.
    let mut failures = Vec::new();
    for (deck, input) in decks() {
        let parsed = parse_deck(&input).unwrap();
        let text = support::run_original(&reference, &input, "wing.txt");
        let cases: Vec<_> = text.split("CASE\n").skip(1).collect();
        assert_eq!(cases.len(), parsed.cases.len(), "{deck}: case count");
        for (index, (case, recorded_text)) in parsed.cases.iter().zip(cases).enumerate() {
            let geometry = Geometry::from_case(case).unwrap();
            let aero = analyze(&geometry).unwrap_or_else(|e| panic!("{deck} case {index}: {e}"));
            let original = snapshots(recorded_text);
            let rust: Vec<_> = aero.points.iter().filter(|p| recorded(p)).collect();
            assert_eq!(original.len(), rust.len(), "{deck} case {index}: points");
            // Other components write their own STB slots; compare only the
            // wing's there (all slots for a wing alone).
            let wing_alone = case.namelist("BODY").is_none_or(|n| !n.present)
                && ["HTPLNF", "VTPLNF", "VFPLNF"]
                    .iter()
                    .all(|name| case.namelist(name).is_none_or(|n| !n.present));
            for (point, (o, r)) in original.iter().zip(rust).enumerate() {
                let at =
                    |name: &str, i: usize| format!("{deck} case {index} point {point} {name}({i})");
                let mut check = |name: &str, i: usize, original: f32, rust: f32| {
                    // UNUSED (1e-30) is within the absolute tolerance of 0,
                    // so unset-versus-set slots are checked explicitly.
                    let unset = (original == UNUSED) != (rust == UNUSED);
                    if unset || !support::is_close(original, rust) {
                        failures.push(format!(
                            "{}: original={original:e}, rudatcom={rust:e}",
                            at(name, i)
                        ));
                    }
                };
                check("MACH", 0, o["POINT"][0], r.flight.mach);
                check("RN", 0, o["POINT"][1], r.flight.reynolds);
                // The original computed a wing here that rudatcom defers.
                let Some(w) = r.wing.as_ref() else {
                    continue;
                };
                compared += 1;
                supersonic += usize::from(r.flight.regime == Regime::Supersonic);
                transonic += usize::from(r.flight.regime == Regime::Transonic);
                let n = w.cl.len();
                for (block, values) in [
                    &w.cd, &w.cl, &w.cm, &w.cn, &w.ca, &w.cla, &w.cma, &w.cyb, &w.cnb, &w.clb,
                ]
                .iter()
                .enumerate()
                {
                    assert_eq!(values.len(), n);
                    for (j, &v) in values.iter().enumerate() {
                        check("WING", block * 20 + j + 1, o["WING"][block * 20 + j], v);
                    }
                }
                let work = &w.work;
                for (name, values) in [
                    ("A", &work.a),
                    ("B", &work.b),
                    ("C", &work.c),
                    ("D", &work.d),
                    ("STB", &work.stb),
                    ("WINGIN", &work.wingin),
                    ("SLG", &work.slg),
                    ("TRA", &work.tra),
                    ("SECOND", &work.second),
                ] {
                    // Subsonic snapshots have no SLG row, only transonic
                    // ones have TRA and SECOND.
                    let Some(row) = o.get(name) else {
                        continue;
                    };
                    for (k, &v) in values.iter().enumerate() {
                        // WINGIN(15) holds the original's Hollerith TYPE name.
                        if name == "WINGIN" && k == 14 {
                            continue;
                        }
                        // WINGIN(64) is the LOGICAL CAMBER, SLG(94) the
                        // LOGICAL DETACH and SECOND(15)/(16) DONE/DOL2
                        // (.TRUE. has the bit pattern 1, a denormal within
                        // the absolute tolerance of 0).
                        let logical = matches!(
                            (name, k),
                            ("WINGIN", 63) | ("SLG", 93) | ("SECOND", 14 | 15)
                        );
                        if logical {
                            if (row[k] != 0.0) != (v != 0.0) {
                                check(name, k + 1, row[k], v);
                            }
                            continue;
                        }
                        // DWASH (M09O11, wing-body-tail downwash; which still needs to implemented)
                        // overwrites A(20) before the probe.
                        if name == "A" && k == 19 && !wing_alone {
                            continue;
                        }
                        if name == "STB" && !wing_alone && v == UNUSED {
                            continue;
                        }
                        check(name, k + 1, row[k], v);
                    }
                }
            }
        }
    }
    assert!(
        failures.is_empty(),
        "{} mismatches:\n{}",
        failures.len(),
        failures[..failures.len().min(2000)].join("\n")
    );
    assert!(compared > 0, "no wing points compared");
    assert!(supersonic > 0, "no supersonic wing points compared");
    assert!(transonic > 0, "no transonic wing points compared");
    eprintln!("compared {compared} wing points ({supersonic} supersonic, {transonic} transonic)");
}
