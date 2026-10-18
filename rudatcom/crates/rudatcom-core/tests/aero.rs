//! Live differential test of the ported aerodynamic methods. The original is
//! rebuilt with control-flow hooks only (no formulas changed) that record
//! its COMMON-block results; nothing from the original is stored.

mod support;

use rudatcom_core::{
    aero::{analyze, BodyAero, BodyMethod},
    geometry::Geometry,
    input::{parse_deck, UNUSED},
};
use std::{collections::HashMap, fs};

/// Build the original with probe hooks: CASE markers at the end of CHECK,
/// a subsonic flag around MAIN00's CALL MAIN01, a body snapshot after
/// MAIN01's M04O04/M06O06 choice, and body-case regime snapshots before
/// MAIN03's closing CALL M12O14 and at the end of MAIN04's NF > 0 path.
fn reference() -> support::Reference {
    let source = support::original_source().replace('\r', "");
    let main01 = "        IF(LOASRT) GO TO 1030\n          CALL MAIN01\n";
    let overlay = "        IF(   FLAG   ) CALL M06O06\n";
    let main03 = "      CALL M12O14\n      DOL2 = .FALSE.\n";
    let main04 = " 1060   CONTINUE\n        CALL M46O56\n 1070 CONTINUE\n";
    for hook in [support::CHECK_END, main01, overlay, main03, main04] {
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
        )
        .replace(
            main03,
            &format!("      IF(BO) CALL RUDATCOM_REGIME_PROBE(2)\n{main03}"),
        )
        .replace(
            main04,
            &format!("{main04}      IF(BO) CALL RUDATCOM_REGIME_PROBE(3)\n"),
        );
    support::compile(&source, include_str!("body_driver.f90"))
}

/// Decks: the 11 samples plus body-specific edge cases.
fn decks() -> Vec<(String, String)> {
    let dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../tests/cases");
    let mut decks: Vec<_> = (1..=11)
        .map(|n| format!("ex{n}.inp"))
        .chain(
            [
                "methods",
                "asymmetric",
                "flight",
                "experimental",
                "supersonic",
                "tails",
                "hypersonic",
                "transonic",
            ]
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

/// One recorded original body snapshot: row name -> values.
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

/// SBD(n) (1-based) values of a SYPBOD/HYPBOD result, as (slot, value).
fn sbd_slots(b: &BodyAero) -> Vec<(usize, f32)> {
    let mut slots = Vec::new();
    let mut run = |first: usize, values: &[f32]| {
        slots.extend(values.iter().enumerate().map(|(j, &v)| (first + j, v)));
    };
    if let Some(s) = &b.supersonic {
        run(
            1,
            &[
                s.nose_afterbody_length,
                s.length,
                s.tail_length,
                s.nose_diameter,
                s.afterbody_diameter,
                s.base_diameter,
                s.beta,
                s.afterbody_fineness,
                s.tail_fineness,
                s.nose_fineness,
                s.nose_xcp,
                s.nose_cylinder_cma,
                s.tail_cma,
                s.boattail_angle,
                s.tail_cna,
                s.flare_angle,
                s.nose_cylinder_cna,
                s.cna,
                s.base_area,
                s.planform_area,
            ],
        );
        run(41, &s.crossflow_mach);
        run(61, &s.crossflow_drag);
        run(81, &s.crossflow);
        run(101, &[s.boattail_xcp, s.flare_angle_cna, s.flare_cmap]);
        run(
            105,
            &[
                s.planform_centroid,
                s.volume,
                s.nose_wave_factor,
                s.nose_wave_drag,
            ],
        );
        run(
            110,
            &[
                s.cma,
                s.wetted_area,
                s.reynolds,
                s.cutoff_reynolds,
                s.skin_friction,
                s.friction_drag,
                s.interference_factor,
                s.interference_drag,
                s.afterbody_wave_factor,
                s.afterbody_wave_drag,
                s.maximum_diameter,
                s.base_drag_factor,
                s.base_pressure,
                s.base_drag,
                s.zero_lift_drag,
            ],
        );
        run(135, &[s.tail_angle]);
    }
    if let Some(h) = &b.hypersonic {
        run(
            1,
            &[
                h.nose_afterbody_length,
                h.length,
                h.tail_length,
                h.nose_diameter,
                h.afterbody_diameter,
                h.base_diameter,
            ],
        );
        run(10, &[h.nose_fineness]);
        run(14, &[h.boattail_angle]);
        run(16, &[h.flare_angle]);
        run(18, &[h.cna]);
        run(102, &[h.flare_angle]);
        run(110, &[h.cma]);
        run(
            125,
            &[
                h.nose_cna_factor,
                h.nose_xcp,
                h.nose_angle,
                h.nose_cna,
                h.nose_cma,
                h.afterbody_angle,
                h.afterbody_cna_factor,
                h.afterbody_cma_factor,
                h.afterbody_cna,
                h.afterbody_cma,
                h.tail_angle,
                h.tail_cna_factor,
                h.tail_cma_factor,
                h.tail_cna,
                h.tail_cma,
                h.newtonian_factor,
            ],
        );
        run(141, &h.surface_angle);
        run(161, &h.moment_arm);
        run(181, &h.cn_integral);
        run(201, &h.cm_integral);
    }
    slots
}

#[test]
fn body_matches_original() {
    let reference = reference();
    let mut compared = [0; 3];
    for (deck, input) in decks() {
        let parsed = parse_deck(&input).unwrap();
        let text = support::run_original(&reference, &input, "body.txt");
        let cases: Vec<_> = text.split("CASE\n").skip(1).collect();
        assert_eq!(cases.len(), parsed.cases.len(), "{deck}: case count");
        for (index, (case, recorded)) in parsed.cases.iter().zip(cases).enumerate() {
            let geometry = Geometry::from_case(case).unwrap();
            let aero = analyze(&geometry).unwrap_or_else(|e| panic!("{deck} case {index}: {e}"));
            let original = snapshots(recorded);
            let rust: Vec<_> = aero
                .points
                .iter()
                .filter(|p| p.body.is_some() || p.deferred)
                .collect();
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
                // The original computed a body here that rudatcom defers.
                let Some(b) = r.body.as_ref() else {
                    continue;
                };
                // KIND: 1 = MAIN01 (subsonic), 2 = MAIN03, 3 = MAIN04.
                let kind = o.get("KIND").map_or(1, |k| k[0] as usize);
                let expected = match b.method {
                    BodyMethod::Transonic => 2,
                    BodyMethod::Supersonic | BodyMethod::Hypersonic => 3,
                    _ => 1,
                };
                assert_eq!(kind, expected, "{}: {:?}", at("KIND", 0), b.method);
                compared[kind - 1] += 1;
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
                if let Some(s) = &b.slender {
                    let drag = [
                        s.length,
                        s.maximum_area,
                        s.base_area,
                        s.fineness_ratio,
                        s.length_reynolds,
                        s.cutoff_reynolds,
                        s.skin_friction,
                        s.wetted_area,
                        s.friction_drag,
                        s.base_drag,
                        s.zero_lift_drag,
                    ];
                    for (i, (&o, r)) in o["DRAG"].iter().zip(drag).enumerate() {
                        check("DRAG", i, o, r);
                    }
                    let m = &o["METHOD"];
                    for (i, v) in [s.apparent_mass_factor, s.eta, s.alpha_zero, s.cm0]
                        .into_iter()
                        .enumerate()
                    {
                        if let Some(v) = v {
                            check("METHOD", i, m[i], v);
                        }
                    }
                    for (j, &v) in s.crossflow_drag.iter().enumerate() {
                        check("CDC", j, m[4 + j], v);
                    }
                }
                if kind == 1 {
                    continue;
                }
                for (slot, v) in sbd_slots(b) {
                    // Transonic SYPBOD leaves the per-alpha slots from other
                    // overlays alone; only compare what this point computed.
                    if kind == 2 && v == UNUSED {
                        continue;
                    }
                    check("SBD", slot, o["SBD"][slot - 1], v);
                }
                if let Some(s) = &b.supersonic {
                    let bds = &o["BDS"];
                    for (first, values) in [
                        (536, &s.equivalent_radius),
                        (556, &s.cn_potential),
                        (576, &s.cn_viscous),
                        (596, &s.cm_potential),
                        (616, &s.cm_viscous),
                    ] {
                        for (j, &v) in values.iter().enumerate() {
                            check("BD", first + j, bds[first - 536 + j], v);
                        }
                    }
                }
            }
        }
    }
    assert!(
        compared.iter().all(|&n| n > 0),
        "points compared per kind: {compared:?}"
    );
}
