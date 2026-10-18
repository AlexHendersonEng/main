//! Live differential test of the subsonic tail methods without a body
//! Horizontal tail, vertical tail, ventral fin and twin
//! vertical panels. The original is rebuilt with control-flow hooks only (no
//! formulas changed) that record the tail COMMON blocks after MAIN01's
//! lateral derivatives; nothing from the original is stored.

mod support;

use rudatcom_core::{
    aero::{analyze, FinAero, PointAero},
    flight::Regime,
    geometry::Geometry,
    input::{parse_deck, UNUSED},
};
use std::{collections::HashMap, fs};

/// Build the original with probe hooks: CASE markers at the end of CHECK, a
/// subsonic flag around MAIN00's CALL MAIN01 and a tail snapshot after
/// MAIN01's M29O35/M17O21 for body-less cases with any tail.
fn reference() -> support::Reference {
    let source = support::original_source().replace('\r', "");
    let main01 = "        IF(LOASRT) GO TO 1030\n          CALL MAIN01\n";
    let lateral = "      CALL M29O35\n      CALL M17O21\n";
    for hook in [support::CHECK_END, main01, lateral] {
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
            &format!(
                "{lateral}      IF(.NOT. BO .AND. (HTPL .OR. VTPL .OR. VFPL .OR. TVTPAN)) CALL RUDATCOM_TAIL_PROBE\n"
            ),
        );
    support::compile(&source, include_str!("tail_driver.f90"))
}

fn decks() -> Vec<(String, String)> {
    let dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../tests/cases/tail");
    ["horizontal", "vertical", "combined"]
        .iter()
        .map(|name| {
            let text = fs::read_to_string(dir.join(format!("{name}.inp"))).unwrap();
            (format!("tail/{name}.inp"), text.replace('\r', ""))
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

/// Points at which the original records tails (subsonic MAIN00 calls of
/// MAIN01; MAIN03's pre-pass calls are not flagged).
fn recorded(p: &PointAero) -> bool {
    p.flight.regime == Regime::Subsonic
        && (p.horizontal_tail.is_some()
            || p.vertical_tail.is_some()
            || p.ventral_fin.is_some()
            || p.tails_deferred)
}

/// Compare one array slot by slot. UNUSED (1e-30) is within the absolute
/// tolerance of 0, so unset-versus-set slots are checked explicitly.
/// `skip` lists 0-based slots not compared (with a reason at the call).
fn compare(
    failures: &mut Vec<String>,
    at: &str,
    name: &str,
    original: &[f32],
    rust: &[f32],
    skip: &[usize],
) {
    for (k, (&o, &r)) in original.iter().zip(rust).enumerate() {
        if skip.contains(&k) {
            continue;
        }
        let unset = (o == UNUSED) != (r == UNUSED);
        if unset || !support::is_close(o, r) {
            failures.push(format!(
                "{at} {name}({}): original={o:e}, rudatcom={r:e}",
                k + 1
            ));
        }
    }
}

/// A fin's IOM block, AVT/AVF, VTIN/VFIN, DVT/DVF and (when its SUBLAT tail
/// section ran) the host STB block.
fn compare_fin(
    failures: &mut Vec<String>,
    at: &str,
    names: [&str; 5],
    o: &HashMap<&str, Vec<f32>>,
    f: &FinAero,
) {
    let [iom, a, ain, d, stb] = names;
    compare(failures, at, iom, &o[iom], &f.work.iom, &[]);
    for (block, values) in [&f.cd, &f.cyb, &f.cnb, &f.clb].iter().enumerate() {
        let base = [0, 140, 160, 180][block];
        compare(failures, at, iom, &o[iom][base..], values, &[]);
    }
    // Twin panels alone have no AVT/VTIN.
    if !f.work.a.is_empty() {
        compare(failures, at, a, &o[a], &f.work.a, &[]);
        // Slot 10 (VTIN's ALPHAO, never stored for VFIN) is not read by
        // the subsonic methods; slot 15 holds the Hollerith TYPE name.
        compare(failures, at, ain, &o[ain], &f.work.ain, &[9, 14]);
    }
    compare(failures, at, d, &o[d], &f.work.d, &[]);
    if let Some(s) = &f.work.stb {
        compare(failures, at, stb, &o[stb], s, &[]);
    }
}

#[test]
fn tails_match_original() {
    let reference = reference();
    let (mut ht, mut vt, mut vf) = (0, 0, 0);
    let mut failures = Vec::new();
    for (deck, input) in decks() {
        let parsed = parse_deck(&input).unwrap();
        let text = support::run_original(&reference, &input, "tail.txt");
        let cases: Vec<_> = text.split("CASE\n").skip(1).collect();
        assert_eq!(cases.len(), parsed.cases.len(), "{deck}: case count");
        for (index, (case, recorded_text)) in parsed.cases.iter().zip(cases).enumerate() {
            let geometry = Geometry::from_case(case).unwrap();
            let aero = analyze(&geometry).unwrap_or_else(|e| panic!("{deck} case {index}: {e}"));
            let original = snapshots(recorded_text);
            let rust: Vec<_> = aero.points.iter().filter(|p| recorded(p)).collect();
            assert_eq!(original.len(), rust.len(), "{deck} case {index}: points");
            for (point, (o, r)) in original.iter().zip(rust).enumerate() {
                let at = format!("{deck} case {index} point {point}");
                assert!(!r.tails_deferred, "{at}: tails deferred");
                compare(
                    &mut failures,
                    &at,
                    "MACH",
                    &o["POINT"][..1],
                    &[r.flight.mach],
                    &[],
                );
                if let Some(h) = &r.horizontal_tail {
                    ht += 1;
                    for (block, values) in [
                        &h.cd, &h.cl, &h.cm, &h.cn, &h.ca, &h.cla, &h.cma, &h.cyb, &h.cnb, &h.clb,
                    ]
                    .iter()
                    .enumerate()
                    {
                        compare(
                            &mut failures,
                            &at,
                            "HT",
                            &o["HT"][block * 20..],
                            values,
                            &[],
                        );
                    }
                    let w = &h.work;
                    for (name, values) in [
                        ("AHT", &w.a),
                        ("BHT", &w.b),
                        ("CHT", &w.c),
                        ("DHT", &w.d),
                        ("STBH", &w.stb),
                    ] {
                        compare(&mut failures, &at, name, &o[name], values, &[]);
                    }
                    // HTIN(15) is the Hollerith TYPE name; HTIN(64) the
                    // LOGICAL CAMBER (.TRUE. = bit pattern 1, a denormal).
                    compare(&mut failures, &at, "HTIN", &o["HTIN"], &w.wingin, &[14, 63]);
                    if (o["HTIN"][63] != 0.0) != (w.wingin[63] != 0.0) {
                        failures.push(format!("{at} HTIN(64): CAMBER differs"));
                    }
                }
                if let Some(f) = &r.vertical_tail {
                    vt += 1;
                    let names = ["VT", "AVT", "VTIN", "DVT", "STB"];
                    compare_fin(&mut failures, &at, names, o, f);
                }
                if let Some(f) = &r.ventral_fin {
                    vf += 1;
                    let names = ["VF", "AVF", "VFIN", "DVF", "STBH"];
                    compare_fin(&mut failures, &at, names, o, f);
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
    assert!(
        ht > 0 && vt > 0 && vf > 0,
        "tails compared: {ht} HT, {vt} VT, {vf} VF"
    );
    eprintln!("compared {ht} HT, {vt} VT and {vf} VF points");
}
