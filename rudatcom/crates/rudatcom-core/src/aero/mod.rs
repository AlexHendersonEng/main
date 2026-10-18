//! Aerodynamic methods, driven per flight point like the original's MAIN00.
//!
//! Ported so far: subsonic body-alone aerodynamics (MAIN01's M04O04/M06O06
//! overlays). Transonic/supersonic bodies and the lifting surfaces follow in
//! later checkpoints; their results are `None` until then.

pub mod body;

use crate::{
    flight::{flight_points, FlightPoint, Regime},
    geometry::Geometry,
    math::MathError,
};
pub use body::{BodyAero, BodyMethod};

#[derive(Debug, thiserror::Error)]
pub enum AeroError {
    #[error("invalid aerodynamic input: {0}")]
    Invalid(String),
    #[error("aerodynamic calculation: {0}")]
    Numerical(#[from] MathError),
}
pub type Result<T> = std::result::Result<T, AeroError>;

/// Results for one MAIN00 call.
#[derive(Debug, Clone)]
pub struct PointAero {
    pub flight: FlightPoint,
    /// Body-alone coefficients (subsonic only for now).
    pub body: Option<BodyAero>,
}

#[derive(Debug, Clone)]
pub struct CaseAero {
    pub points: Vec<PointAero>,
}

/// Run the ported methods over the case's flight points, in the original's
/// LOOP order. Cases are independent: INITZE resets the work arrays.
pub fn analyze(geometry: &Geometry) -> Result<CaseAero> {
    let case = &geometry.inputs.case;
    let present = |name| case.namelist(name).is_some_and(|n| n.present);
    let mut points = Vec::new();
    for flight in flight_points(case)? {
        // MAIN00 runs only the TRNJET/hypersonic overlays for TRAJET, and
        // LOASRT (LARWB) bypasses MAIN01 entirely.
        let body = if flight.regime == Regime::Subsonic
            && geometry.body.is_some()
            && !present("TRNJET")
            && !present("LARWB")
        {
            // EXPDAT: measured data for this Mach index replaces results.
            let experimental = case
                .experimental(flight.mach_index + 1)
                .map_err(|e| AeroError::Invalid(format!("experimental data: {e}")))?;
            Some(body::subsonic(geometry, &flight, experimental.as_ref())?)
        } else {
            None
        };
        points.push(PointAero { flight, body });
    }
    Ok(CaseAero { points })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::input::parse_deck;

    fn run(deck: &str) -> Result<CaseAero> {
        let parsed = parse_deck(deck).unwrap();
        analyze(&Geometry::from_case(&parsed.cases[0]).unwrap())
    }

    const FLIGHT: &str = " $FLTCON NMACH=2.,MACH(1)=0.3,1.5,NALPHA=2.,ALSCHD(1)=0.,5.,\n \
                          RNNUB(1)=1.E6,1.E6$\n";
    const BODY: &str = " $BODY NX=3.,X(1)=0.,2.,4.,R(1)=0.,0.5,0.5$\n";

    #[test]
    fn body_without_reference_area_is_an_error() {
        let deck = format!("{FLIGHT} $OPTINS CBARR=1.$\n{BODY}");
        assert!(matches!(run(&deck), Err(AeroError::Invalid(_))));
    }

    #[test]
    fn only_subsonic_points_have_body_results_so_far() {
        let deck = format!("{FLIGHT} $OPTINS SREF=1.,CBARR=1.$\n{BODY}");
        let points = run(&deck).unwrap().points;
        assert_eq!(points.len(), 2);
        assert_eq!(
            points[0].body.as_ref().unwrap().method,
            BodyMethod::Axisymmetric
        );
        assert!(points[1].body.is_none());
    }
}
