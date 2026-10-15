//! Unit tests for the input reader (no original DATCOM needed; see tests/reference.rs
//! and tests/input_vs_reference.rs for the comparisons against the original).

use super::namelist::{to_dec, to_int, to_log};
use super::*;

fn num(s: &str) -> f32 {
    to_dec(s.as_bytes()).0
}

#[test]
fn to_dec_parses_fortran_numbers() {
    assert_eq!(num("1.0"), 1.0);
    assert_eq!(num("  -6.0 "), -6.0);
    assert_eq!(num("+.5"), 0.5);
    assert_eq!(num("6.4E6"), 6.4e6);
    assert_eq!(num("1.5E-3"), 1.5e-3);
    assert_eq!(num(""), 0.0);
    assert!(to_dec(b"1.0X").1, "letters are a type error");
    assert!(to_dec(b".TRUE.").1);
}

#[test]
fn to_int_rejects_decimal_point() {
    assert_eq!(to_int(b"3"), (3, false));
    assert!(to_int(b"3.0").1);
}

#[test]
fn to_log_accepts_short_and_long_forms() {
    assert_eq!(to_log(b" .TRUE."), Some(true));
    assert_eq!(to_log(b".T."), Some(true));
    assert_eq!(to_log(b".FALSE."), Some(false));
    assert_eq!(to_log(b".F."), Some(false));
    assert_eq!(to_log(b"TRUE"), None);
    assert_eq!(to_log(b".TRU."), None);
}

fn one_case(deck: &str) -> Case {
    let d = parse_deck(deck).expect("parse");
    assert_eq!(d.cases.len(), 1);
    d.cases.into_iter().next().unwrap()
}

#[test]
fn reads_scalars_arrays_and_continuations() {
    let c = one_case(
        " $FLTCON NMACH=3.0, MACH(1)=0.9,1.4,2.5, RNNUB=6.4E6,\n 9.96E6, 17.0E6$\n NEXT CASE\n"
            .replace(" NEXT", "NEXT")
            .as_str(),
    );
    let f = c.namelist("FLTCON").unwrap();
    assert!(f.present);
    assert_eq!(f.scalar("NMACH"), Some(3.0));
    let m = f.get("MACH").unwrap();
    assert!((m[0] - 0.9).abs() < 1e-6 && m[1] == 1.4 && m[2] == 2.5 && m[3] == UNUSED);
    assert_eq!(&f.get("RNNUB").unwrap()[..3], &[6.4e6, 9.96e6, 17.0e6]);
    assert!(!c.regimes.subsonic && c.regimes.supersonic && c.regimes.transonic);
}

#[test]
fn repeat_counts_and_aliases() {
    let c = one_case(" $FLTCON NMACH=1.0, MACH(1)=0.6, NALPHA=4.0, ALPHA(2)=3*5.0$\nNEXT CASE\n");
    let f = c.namelist("FLTCON").unwrap();
    // ALPHA is an alias of ALSCHD.
    assert_eq!(
        &f.get("ALSCHD").unwrap()[..5],
        &[UNUSED, 5.0, 5.0, 5.0, UNUSED]
    );
}

#[test]
fn logicals_and_clamps() {
    let c = one_case(" $FLTCON NMACH=1.0, MACH(1)=2.5, HYPERS=.TRUE., STMACH=0.1, TSMACH=9.0$\n");
    let f = c.namelist("FLTCON").unwrap();
    assert!(f.flag("HYPERS"));
    assert_eq!(f.scalar("STMACH"), Some(0.6));
    assert_eq!(f.scalar("TSMACH"), Some(1.4));
}

#[test]
fn body_radius_derives_area_and_perimeter() {
    let c = one_case(" $BODY NX=2.0, X(1)=0.0,1.0, R(1)=1.0,1.0$\n");
    let b = c.namelist("BODY").unwrap();
    let s = b.get("S").unwrap()[0];
    assert!((s - std::f32::consts::PI).abs() < 1e-5);
    assert!(c.is_runnable());
}

#[test]
fn body_with_only_perimeter_is_an_error() {
    let c = one_case(" $BODY NX=2.0, X(1)=0.0,1.0, P(1)=1.0,1.0$\n");
    assert!(!c.is_runnable());
}

#[test]
fn control_cards_set_flags() {
    let c =
        one_case("CASEID MY TEST\nDIM M\nDERIV RAD\nSAVE\nTRIM\nDUMP CASE\n $OPTINS SREF=1.0$\n");
    assert_eq!(c.id.as_deref(), Some(" MY TEST"));
    assert_eq!(c.dim, DimUnit::Meters);
    assert!(c.controls.save && c.controls.trim && c.controls.dump_case);
    assert_eq!(c.controls.deriv_rad, Some(true));
}

#[test]
fn unknown_control_card_blocks_the_case() {
    let c = one_case("BOGUS CARD\n $OPTINS SREF=1.0$\n");
    assert!(!c.is_runnable());
    assert_eq!(c.diagnostics[0].line, 1);
}

#[test]
fn save_carries_namelists_to_next_case() {
    let d = parse_deck("SAVE\n $OPTINS SREF=1.0$\nNEXT CASE\n $OPTINS CBARR=2.0$\nNEXT CASE\n")
        .unwrap();
    assert_eq!(d.cases.len(), 2);
    let o = d.cases[1].namelist("OPTINS").unwrap();
    assert_eq!(o.scalar("SREF"), Some(1.0));
    assert_eq!(o.scalar("CBARR"), Some(2.0));
    // Without SAVE nothing carries over.
    let d = parse_deck(" $OPTINS SREF=1.0$\nNEXT CASE\n $OPTINS CBARR=2.0$\nNEXT CASE\n").unwrap();
    assert_eq!(d.cases[1].namelist("OPTINS").unwrap().scalar("SREF"), None);
}

#[test]
fn crlf_input_is_accepted() {
    let c = one_case(" $OPTINS SREF=1.0,\r\n CBARR=2.0$\r\nNEXT CASE\r\n");
    assert_eq!(c.namelist("OPTINS").unwrap().scalar("CBARR"), Some(2.0));
}

#[test]
fn syntax_errors_carry_line_numbers() {
    let e = parse_deck(" $OPTINS SREF=1.0$\nNEXT CASE\n $OPTINS\n BOGUS=1.0$\n").unwrap_err();
    match e {
        crate::Error::Parse { line, message, .. } => {
            assert_eq!(line, 4);
            assert!(message.contains("NOT IN NAMELIST"), "{message}");
        }
        other => panic!("unexpected {other:?}"),
    }
    assert!(parse_deck(" $OPTINS SREF 1.0$\n").is_err());
    assert!(parse_deck(" $OPTINS SREF=ABC$\n").is_err());
    assert!(parse_deck(" $FLTCON MACH(21)=1.0$\n").is_err());
}

#[test]
fn every_sample_deck_parses_cleanly() {
    let dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("../../tests/cases");
    let mut n = 0;
    for entry in std::fs::read_dir(dir).unwrap() {
        let p = entry.unwrap().path();
        if p.extension().is_some_and(|e| e == "inp") {
            let deck = parse_deck(&std::fs::read_to_string(&p).unwrap())
                .unwrap_or_else(|e| panic!("{}: {e}", p.display()));
            for c in &deck.cases {
                assert!(c.is_runnable(), "{}: {:?}", p.display(), c.diagnostics);
            }
            n += 1;
        }
    }
    assert!(n >= 11);
}
