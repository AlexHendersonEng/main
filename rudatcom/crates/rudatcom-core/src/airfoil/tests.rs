//! Focused airfoil checks; live parity with M50O62 is in tests/numerics.rs.

use super::aero::asmint;
use super::*;
use crate::input::parse_deck;

/// Converted case with the airfoil module applied.
fn sections(input: &str) -> Case {
    let deck = parse_deck(input).unwrap();
    let mut case = crate::geometry::convert_case(&deck.cases[0]).unwrap().case;
    apply_sections(&mut case).unwrap();
    case
}

fn schr(case: &Case, name: &str, var: &str) -> f32 {
    case.namelist(name).unwrap().get(var).unwrap()[0]
}

const PLANFORM: &str = " $WGPLNF CHRDR=3.,CHRDTP=1.,SSPN=4.,SSPNE=4.,SAVSI=0.$\n";
const FLIGHT: &str = " $FLTCON NMACH=2.,MACH=0.2,0.6,RNNUB=2.E6,2.E6$\n";

#[test]
fn asmint_returns_nodes_and_reproduces_a_parabola() {
    let x = [0.0, 0.25, 0.5, 0.75, 1.0];
    let y = x.map(|v: f32| v * v);
    assert_eq!(asmint(&x, &y, 0.5).unwrap(), 0.25);
    assert!((asmint(&x, &y, 0.6).unwrap() - 0.36).abs() < 0.01);
    assert!(asmint(&x[..3], &y[..3], 0.5).is_err());
}

#[test]
fn naca_0012_fills_unspecified_section_inputs() {
    let case = sections(&format!("{FLIGHT}{PLANFORM}NACA-W-4-0012\n"));
    assert!((schr(&case, "WGSCHR", "TOVC") - 0.12).abs() < 1e-4);
    assert!((schr(&case, "WGSCHR", "XOVC") - 0.3).abs() < 0.02);
    // A symmetric section has no camber, ideal lift or zero-lift angle.
    assert_eq!(schr(&case, "WGSCHR", "CAMBER"), 0.0);
    assert!(schr(&case, "WGSCHR", "CLI").abs() < 1e-4);
    assert!(schr(&case, "WGSCHR", "ALPHAO").abs() < 1e-3);
    // Thin-airfoil lift slope is about 0.1 per degree.
    let cla = case.namelist("WGSCHR").unwrap().get("CLALPA").unwrap();
    assert!((0.09..0.12).contains(&cla[0]), "{}", cla[0]);
    assert!(cla[1] > cla[0], "compressibility raises the slope");
}

#[test]
fn user_values_are_not_overwritten() {
    let case = sections(&format!(
        "{FLIGHT}{PLANFORM} $WGSCHR TOVC=0.2,CLALPA=0.05$\nNACA-W-4-2412\n"
    ));
    assert_eq!(schr(&case, "WGSCHR", "TOVC"), 0.2);
    // The emulated namelist number reader does not round 0.05 correctly.
    assert!((schr(&case, "WGSCHR", "CLALPA") - 0.05).abs() < 1e-6);
    assert_eq!(schr(&case, "WGSCHR", "CAMBER"), 1.0);
    assert!(schr(&case, "WGSCHR", "ALPHAO") < 0.0);
}

#[test]
fn surfaces_without_sections_are_left_alone() {
    let case = sections(&format!("{FLIGHT}{PLANFORM}"));
    assert_eq!(schr(&case, "WGSCHR", "TOVC"), UNUSED);
}

#[test]
fn ventral_fin_alphao_is_stored_with_the_vertical_tail() {
    let deck = parse_deck(" $VFSCHR ALPHAO=-2.$\n").unwrap();
    let case = &deck.cases[0];
    assert_eq!(schr(case, "VTSCHR", "ALPHAO"), -2.0);
}

#[test]
fn tail_without_type_gets_outboard_panel_values() {
    // CHECK leaves a tail with no TYPE matching no planform name, so SECO
    // treats it as non-straight-tapered; TYPE=1 is straight.
    let fin = " $VTPLNF CHRDR=3.,CHRDTP=1.,SSPN=3.,SSPNE=3.,SAVSI=30.";
    let missing = sections(&format!("{FLIGHT}{PLANFORM}{fin}$\nNACA-V-4-0010\n"));
    assert!((schr(&missing, "VTSCHR", "TOVCO") - 0.1).abs() < 1e-4);
    let straight = sections(&format!(
        "{FLIGHT}{PLANFORM}{fin},TYPE=1.$\nNACA-V-4-0010\n"
    ));
    assert_eq!(schr(&straight, "VTSCHR", "TOVCO"), UNUSED);
}

#[test]
fn invalid_typein_is_an_error() {
    let deck = parse_deck(&format!(
        "{FLIGHT}{PLANFORM} $WGSCHR TYPEIN=3.,NPTS=4.,XCORD=0.,.3,.6,1.$\n"
    ))
    .unwrap();
    let mut case = crate::geometry::convert_case(&deck.cases[0]).unwrap().case;
    assert!(apply_sections(&mut case).is_err());
}
