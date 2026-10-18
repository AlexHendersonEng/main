//! Aerodynamic methods, driven per flight point like the original's MAIN00.
//!
//! Ported so far: body-alone aerodynamics in every regime (MAIN01's
//! M04O04/M06O06, MAIN03's transonic body path and MAIN04's M19O23/M26O32)
//! and the subsonic and supersonic wing alone (MAIN01's and MAIN04's wing
//! overlays, [`wing`]). The other lifting-surface methods follow in later
//! checkpoints.

pub mod body;
pub mod supersonic;
pub mod wing;

use crate::{
    flight::{flight_points, FlightPoint, Regime},
    geometry::Geometry,
    input::UNUSED,
    math::MathError,
};
pub use body::{BodyAero, BodyMethod, SlenderBody};
pub use supersonic::{HypersonicBody, SupersonicBody};
pub use wing::{WingAero, WingWork};

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
    /// Body-alone coefficients, when the original computes them.
    pub body: Option<BodyAero>,
    /// The original computes body results here that depend on methods not
    /// yet ported (MAIN03's DOL2 pre-pass with lifting surfaces); `body` is
    /// `None` rather than an approximation.
    pub deferred: bool,
    /// Wing-alone coefficients (subsonic, and supersonic without a body).
    pub wing: Option<WingAero>,
    /// The original computes wing results here with methods not yet ported
    /// (transonic wing, supersonic wing with a body or power, or a point
    /// after one of those, a bypassed point or a subsonic point following a
    /// supersonic one, whose persistent wing state is then unknown).
    pub wing_deferred: bool,
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
    let fl = case.namelist("FLTCON").expect("FLTCON schema");
    let hypers = fl.flag("HYPERS");
    // MAIN04 skips the body for ground effect (GRNDEF NGH, the original's
    // FLC(63)) and for propeller or jet power.
    let powered = case
        .namelist("GRNDEF")
        .is_some_and(|g| g.values[62] != UNUSED)
        || present("PROPWR")
        || present("JETPWR");
    let method = case
        .namelist("BODY")
        .map_or(0, |b| (b.values[127] + 0.5) as i32);
    // MAIN03's DOL2: every lifting surface (wing, horizontal tail) present
    // is straight tapered (TYPE 1).
    let straight_or_absent = |name| {
        case.namelist(name)
            .is_none_or(|n| !n.present || n.scalar("TYPE") == Some(1.0))
    };
    let dol2 = straight_or_absent("WGPLNF") && straight_or_absent("HTPLNF");
    let lifting = present("WGPLNF") || present("HTPLNF");
    // SETUP2's DONE: the DOL2 pre-pass runs once per case (INITZE resets it).
    let mut done = false;
    let flights = flight_points(case)?;
    // Per-case wing state; the CLMCH0 pre-pass runs here. A numerical
    // failure there (e.g. CLAMO unset with transonic Machs) defers the wing
    // for the whole case rather than reporting the original's inf/NaN.
    let (mut wing_state, wing_present) = match wing::WingCase::new(geometry, &flights) {
        Ok(state) => {
            let present = state.is_some();
            (state, present)
        }
        Err(AeroError::Numerical(_)) => (None, true),
        Err(e) => return Err(e),
    };
    let mut points = Vec::new();
    let mut seen_supersonic = false;
    for flight in flights {
        // MAIN00 runs only the hypersonic-flap (HYPEFF, HYPERS, Mach >= 5) or
        // TRNJET overlays at such points, and LOASRT (LARWB) bypasses the
        // regime overlays entirely.
        let hypersonic_flap = present("HYPEFF") && hypers && flight.mach >= 5.0;
        let bypass = hypersonic_flap || present("TRNJET") || present("LARWB");
        // The first transonic MAIN03 call runs the DOL2 pre-pass, body or not.
        let prepass = !bypass && flight.regime == Regime::Transonic && dol2 && !done;
        done |= prepass;
        let deferred = prepass && lifting && geometry.body.is_some();
        // EXPDAT: measured data for this Mach index replaces results.
        let experimental = case
            .experimental(flight.mach_index + 1)
            .map_err(|e| AeroError::Invalid(format!("experimental data: {e}")))?;
        let experimental = experimental.as_ref();
        // The wing state stays trustworthy only through the leading run of
        // subsonic points followed by supersonic ones (M18O22 re-derives A
        // per supersonic point). Supersonic wings with a body or power
        // await SUPLAT's wing-body part and the power methods.
        let supersonic_ok =
            flight.regime == Regime::Supersonic && !powered && geometry.body.is_none();
        match flight.regime {
            _ if bypass => wing_state = None,
            Regime::Subsonic if seen_supersonic => wing_state = None,
            Regime::Subsonic => {}
            Regime::Supersonic if supersonic_ok => seen_supersonic = true,
            _ => wing_state = None,
        }
        let wing = match wing_state.as_mut() {
            Some(state) if flight.regime == Regime::Supersonic => {
                Some(state.supersonic(&flight, experimental)?)
            }
            Some(state) => Some(state.subsonic(&flight, experimental)?),
            None => None,
        };
        let wing_deferred = wing_present && wing.is_none() && !bypass;
        let body = if geometry.body.is_none() || bypass || deferred {
            None
        } else {
            match flight.regime {
                Regime::Subsonic => Some(body::subsonic(geometry, &flight, experimental)?),
                Regime::Transonic => {
                    // Without lifting surfaces the pre-pass only leaves B(1)
                    // = TRAH(6) = UNUSED; with them it runs whole subsonic
                    // and supersonic analyses at substitute Mach numbers,
                    // which await the wing checkpoints (`deferred`).
                    let b1 = if prepass { UNUSED } else { flight.mach };
                    Some(body::transonic(geometry, &flight, experimental, b1)?)
                }
                Regime::Supersonic if powered => None,
                Regime::Supersonic => Some(body::supersonic(
                    geometry,
                    &flight,
                    experimental,
                    hypers && method == 1,
                )?),
            }
        };
        points.push(PointAero {
            flight,
            body,
            deferred,
            wing,
            wing_deferred,
        });
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
    fn every_regime_has_body_results() {
        let deck = format!("{FLIGHT} $OPTINS SREF=1.,CBARR=1.$\n{BODY} $BODY BLN=2.,BLA=2.$\n");
        let points = run(&deck).unwrap().points;
        assert_eq!(points.len(), 2);
        assert_eq!(
            points[0].body.as_ref().unwrap().method,
            BodyMethod::Axisymmetric
        );
        assert_eq!(
            points[1].body.as_ref().unwrap().method,
            BodyMethod::Supersonic
        );
    }

    /// A subsonic point after a supersonic one sees MAIN04-modified wing
    /// state that is not modelled, so it is deferred.
    #[test]
    fn subsonic_wing_after_supersonic_is_deferred() {
        let deck = " $FLTCON NMACH=3.,MACH(1)=0.4,2.0,0.5,NALPHA=2.,ALSCHD(1)=0.,4.,\n \
                    RNNUB(1)=1.E6,1.E6,1.E6$\n $OPTINS SREF=12.,CBARR=2.2$\n \
                    $WGPLNF CHRDTP=1.,SSPNE=3.,SSPN=3.,CHRDR=3.,SAVSI=45.,CHSTAT=0.,TYPE=1.$\n \
                    $WGSCHR TOVC=0.05,CLALPA(1)=0.1,CLMAX(1)=0.9$\n";
        let points = run(deck).unwrap().points;
        let state: Vec<_> = points
            .iter()
            .map(|p| (p.wing.is_some(), p.wing_deferred))
            .collect();
        assert_eq!(state, [(true, false), (true, false), (false, true)]);
    }
}
