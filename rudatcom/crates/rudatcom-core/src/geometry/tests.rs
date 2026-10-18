//! Focused geometry safety and ownership checks; live parity is in numerics.rs.

use super::*;
use crate::input::{parse_deck, DimUnit};

#[test]
fn geometry_converts_a_copy_and_defaults_wing_references() {
    let deck = parse_deck("DIM IN\n $SYNTHS XCG=24., XW=12., SCALE=2.$\n $WGPLNF CHRDR=24., CHRDTP=12., SSPNE=36., SSPN=48., CHSTAT=0.$\n").unwrap();
    let case = &deck.cases[0];
    let g = Geometry::from_case(case).unwrap();
    assert_eq!(case.dim, DimUnit::Inches);
    assert_eq!(list(case, "WGPLNF").scalar("CHRDR"), Some(24.0));
    assert_eq!(g.wing.as_ref().unwrap().planform.root_chord, 4.0);
    assert_eq!(g.reference.span, Some(16.0));
    assert_eq!(g.reference.area, Some(g.wing.as_ref().unwrap().area()));
    assert_eq!(
        g.reference.chord,
        Some(g.wing.as_ref().unwrap().mean_aerodynamic_chord())
    );
    assert_eq!(g.reference.roughness, 1.6e-4);
}

#[test]
fn rejects_invalid_geometry_instead_of_nan_results() {
    for input in [
        " $WGPLNF CHRDR=0., CHRDTP=1., SSPN=2., SSPNE=2.$\n",
        " $WGPLNF CHRDR=2., CHRDTP=1., SSPN=1., SSPNE=2.$\n",
        " $BODY NX=2., X=0.,0., R=1.,1.$\n",
        " $BODY NX=21., X=0.,1., R=1.,1.$\n",
        " $BODY NX=2., X=0.,1., R=0.,0.$\n",
        " $OPTINS SREF=-1.$\n",
    ] {
        let d = parse_deck(input).unwrap();
        assert!(Geometry::from_case(&d.cases[0]).is_err(), "{input}");
    }
}

#[test]
fn save_cases_are_not_scaled_twice() {
    let d = parse_deck("DIM M\n $SYNTHS SCALE=0.5$\n $WGPLNF CHRDR=3., CHRDTP=1., SSPNE=2., SSPN=2.$\nSAVE\nNEXT CASE\n $SYNTHS XCG=1.$\nNEXT CASE\n").unwrap();
    let first = Geometry::from_case(&d.cases[0]).unwrap();
    let second = Geometry::from_case(&d.cases[1]).unwrap();
    assert_eq!(first.wing.unwrap().area(), second.wing.unwrap().area());
}

#[test]
fn single_station_eqspce_repeats_the_station() {
    let b = BodyStations {
        x: vec![2.0],
        radius: vec![1.0],
        area: vec![3.0],
        perimeter: vec![6.0],
    };
    let result = body::eqspce(&b, 5).unwrap();
    assert_eq!(result.stations.x, [2.0; 5]);
    assert_eq!(result.area_derivative, [0.0; 5]);
}

#[test]
fn naca_section_work_is_explicitly_pending() {
    let deck =
        parse_deck("NACA-W-4-0012\n $WGPLNF CHRDR=3.,CHRDTP=1.,SSPN=2.,SSPNE=2.$\n").unwrap();
    let geometry = Geometry::from_case(&deck.cases[0]).unwrap();
    assert_eq!(geometry.pending_airfoils, ['W']);
}

#[test]
fn larwb_reference_overrides_follow_input_order_and_scale() {
    for (input, expected) in [
        (
            " $OPTINS SREF=9.,CBARR=8.$\n $LARWB SREF=3.,L=4.$\n",
            (12., 8.),
        ),
        (
            " $LARWB SREF=3.,L=4.$\n $OPTINS SREF=9.,CBARR=8.$\n",
            (36., 16.),
        ),
    ] {
        let deck = parse_deck(&format!(" $SYNTHS SCALE=2.$\n{input}")).unwrap();
        let geometry = Geometry::from_case(&deck.cases[0]).unwrap();
        assert_eq!(geometry.reference.area, Some(expected.0));
        assert_eq!(geometry.reference.chord, Some(expected.1));
    }
}

#[test]
fn flap_span_correction_is_visible_and_leaves_parsed_input_alone() {
    let deck = parse_deck(
        "DIM IN\n $WGPLNF CHRDR=3.,CHRDTP=1.,SSPN=24.,SSPNE=12.$\n $SYMFLP SPANFI=6.$\n",
    )
    .unwrap();
    let converted = convert_case(&deck.cases[0]).unwrap();
    assert_eq!(list(&converted.case, "SYMFLP").scalar("SPANFI"), Some(1.0));
    assert_eq!(list(&deck.cases[0], "SYMFLP").scalar("SPANFI"), Some(6.0));
    assert!(converted
        .case
        .warnings
        .iter()
        .any(|d| d.message.contains("flap inboard span")));
}
