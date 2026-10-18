//! Body-alone aerodynamics for every speed regime.
//!
//! Subsonic (MAIN01's overlays):
//! * M06O06 -> BODYRT (axisymmetric, DATCOM 4.2.1.1/4.2.1.2 slender-body
//!   lift with Allen-Perkins crossflow) and, for METHOD = 2, BODYJM
//!   (Jorgensen's method).
//! * M04O04 -> BODOPT (asymmetric/cambered bodies, used when ZL is input
//!   and METHOD = 1).
//!
//! Transonic (MAIN03): BODYRT's slopes only, then the M24O30/CNCA slot
//! clean-up. Supersonic (MAIN04): SYPBOD via M19O23, or HYPBOD via M26O32
//! for HYPERS with METHOD = 1 (see [`super::supersonic`]).
//!
//! BODY(n)/BD(n) in comments are the original COMMON slots, to trace results
//! against the Fortran.
// Figure data are copied digit-for-digit from the original DATA statements.
#![allow(clippy::excessive_precision, clippy::approx_constant)]

use super::{
    supersonic::{self, ellipse_factors, HypersonicBody, SupersonicBody},
    AeroError, Result,
};
use crate::{
    flight::{FlightPoint, Regime},
    geometry::{
        body::{eqspce, eqspce_signed},
        BodyStations, Geometry,
    },
    input::{Namelist, UNUSED},
    interp::{self, Extrapolation, Limits},
    math::{finite, powr, trapz},
    tables::{PI, RAD},
};

/// Which original subroutine produced the body coefficients.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BodyMethod {
    /// BODYRT (METHOD = 1 without ZL).
    Axisymmetric,
    /// BODYRT followed by BODYJM (METHOD = 2).
    Jorgensen,
    /// BODOPT (ZL input, METHOD = 1).
    Asymmetric,
    /// MAIN03: BODYRT slopes at alpha(1) only (any METHOD).
    Transonic,
    /// MAIN04 -> M19O23 -> SYPBOD.
    Supersonic,
    /// MAIN04 -> M26O32 -> HYPBOD (FLTCON HYPERS with METHOD = 1).
    Hypersonic,
}

/// Body-alone results at one flight point; per-alpha vectors follow ALSCHD.
///
/// Values the original leaves unset keep its markers: `UNUSED` (INITZ1's
/// fill) or `-UNUSED` (M24O30's "not computed" mark at transonic points).
/// Test with `v.abs() == UNUSED`, like the original's printing code.
#[derive(Debug, Clone)]
pub struct BodyAero {
    pub method: BodyMethod,
    /// BODY(1..), BODY(21..), BODY(41..), BODY(61..), BODY(81..).
    pub cd: Vec<f32>,
    pub cl: Vec<f32>,
    pub cm: Vec<f32>,
    pub cn: Vec<f32>,
    pub ca: Vec<f32>,
    /// Per-degree derivatives BODY(101..), (121..), (141..), (161..), (181..).
    pub cla: Vec<f32>,
    pub cma: Vec<f32>,
    pub cyb: Vec<f32>,
    pub cnb: Vec<f32>,
    pub clb: Vec<f32>,
    /// BODYRT/BODOPT work values (subsonic and transonic points).
    pub slender: Option<SlenderBody>,
    /// SYPBOD work values (supersonic points; transonic points too unless
    /// the original's would be non-finite, e.g. without BLN).
    pub supersonic: Option<SupersonicBody>,
    /// HYPBOD work values (hypersonic points).
    pub hypersonic: Option<HypersonicBody>,
}

/// BODYRT/BODOPT drag and method values (COMMON /BDATA/). At transonic
/// points BODYRT stops after the drag, so only `apparent_mass_factor` of the
/// method values is set.
#[derive(Debug, Clone)]
pub struct SlenderBody {
    /// BD(1): body length.
    pub length: f32,
    /// BD(56): maximum cross-section area.
    pub maximum_area: f32,
    /// BD(57): base area, raised to 30% of the maximum for boat tails.
    pub base_area: f32,
    /// BD(75): fineness ratio from the maximum-area diameter.
    pub fineness_ratio: f32,
    /// BD(90)/BD(91): length and cutoff (roughness) Reynolds numbers.
    pub length_reynolds: f32,
    pub cutoff_reynolds: f32,
    /// BD(92): turbulent flat-plate skin-friction coefficient (FIG26).
    pub skin_friction: f32,
    /// BD(93): wetted area.
    pub wetted_area: f32,
    /// BD(59)/BD(60)/BD(61): friction, base and zero-lift drag.
    pub friction_drag: f32,
    pub base_drag: f32,
    pub zero_lift_drag: f32,
    /// BD(9): apparent-mass factor k2-k1 (axisymmetric methods).
    pub apparent_mass_factor: Option<f32>,
    /// BD(76): crossflow drag proportionality factor eta (axisymmetric).
    pub eta: Option<f32>,
    /// BD(135..): crossflow drag coefficients (axisymmetric).
    pub crossflow_drag: Vec<f32>,
    /// BD(81): zero-lift angle, degrees (BODOPT only).
    pub alpha_zero: Option<f32>,
    /// BD(62): zero-lift pitching moment (BODOPT only).
    pub cm0: Option<f32>,
}

const HOLD: Limits = Limits {
    lower: Extrapolation::Hold,
    upper: Extrapolation::Hold,
};
const fn limits(lower: Extrapolation, upper: Extrapolation) -> Limits {
    Limits { lower, upper }
}

// FIG 4.1.5.1-27 roughness cutoff exponent vs Mach.
const X27M: [f32; 4] = [0.0, 1.0, 2.0, 3.0];
const X27I: [f32; 4] = [1.57780, 1.67221, 1.98509, 2.28874];
// FIG 4.2.1.2-35B crossflow drag vs Mach normal to the axis.
const X1217B: [f32; 14] = [
    0., 0.2, 0.3, 0.36, 0.4, 0.5, 0.6, 0.7, 0.77, 0.8, 0.86, 0.9, 0.98, 1.,
];
const Y1217B: [f32; 14] = [
    1.2, 1.2, 1.21, 1.23, 1.27, 1.36, 1.5, 1.67, 1.75, 1.77, 1.8, 1.8, 1.8, 1.79,
];
// FIG 4.2.1.2-35A eta vs fineness ratio.
const X1217A: [f32; 8] = [2., 4., 8., 12., 16., 20., 24., 28.];
const Y1217A: [f32; 8] = [0.56, 0.6, 0.66, 0.71, 0.74, 0.76, 0.775, 0.79];
// FIG 4.2.1.1-20 apparent-mass factor k2-k1 vs fineness ratio.
const X21120: [f32; 10] = [4., 5., 6., 8., 10., 12., 14., 16., 18., 20.];
const Y21120: [f32; 10] = [
    0.77, 0.825, 0.865, 0.91, 0.94, 0.955, 0.965, 0.97, 0.973, 0.975,
];
// BODOPT figures BA-1 (Kp) and BA-2 (Kv) vs body aspect ratio.
const XBA: [f32; 9] = [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0];
const YBA1: [f32; 9] = [0.0, 0.72, 1.33, 1.83, 2.225, 2.575, 2.88, 3.125, 3.37];
const YBA2: [f32; 9] = [3.14, 3.14, 3.15, 3.19, 3.21, 3.626, 3.31, 3.9, 3.47];
// FIG 4.2.1.2-37 vortex onset angle: X1 = width/depth ratio, X2 = fineness
// ratio; stored X2-fastest as in TLINEX's Y(NX2, NX1).
const X1BA3: [f32; 5] = [1., 2., 3., 4., 100.];
const X2BA3: [f32; 12] = [0., 1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 100.];
#[rustfmt::skip]
const YBA3: [f32; 60] = [
    34.85, 31.20, 27.55, 23.90, 20.25, 16.60, 12.95, 11.25, 10.30, 9.75, 9.40, 0.,
    30.65, 25.90, 21.15, 16.40, 11.65, 9.10, 7.60, 6.70, 6.10, 5.70, 5.50, 0.,
    26.70, 20.00, 13.30, 9.10, 7.50, 4.90, 4.00, 3.50, 3.10, 2.90, 2.75, 0.,
    22.70, 16.00, 9.30, 7.20, 5.00, 3.95, 3.00, 2.40, 2.05, 1.85, 1.70, 0.,
    0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
];

/// Body inputs (BODYI after CONV) plus the reference values the methods use.
/// Scalars the case omits keep `UNUSED`, as in the original.
pub(super) struct Inputs<'a> {
    pub(super) x: &'a [f32],
    pub(super) s: &'a [f32],
    pub(super) p: &'a [f32],
    pub(super) r: &'a [f32],
    pub(super) zu: &'a [f32],
    pub(super) zl: &'a [f32],
    /// BNOSE/BTAIL shape codes (1 = cone, 2 = ogive), nose and afterbody
    /// lengths BLN/BLA and the hypersonic nose bluntness diameter DS.
    pub(super) bnose: f32,
    pub(super) btail: f32,
    pub(super) bln: f32,
    pub(super) bla: f32,
    pub(super) ds: f32,
    pub(super) method: f32,
    pub(super) ellip: f32,
    pub(super) alphas: &'a [f32],
    pub(super) sref: f32,
    pub(super) cbarr: f32,
    pub(super) blref: f32,
    pub(super) roughness: f32,
    pub(super) xcg: f32,
}

impl<'a> Inputs<'a> {
    /// Collect the converted BODY/FLTCON/SYNTHS values for one case.
    fn new(geometry: &'a Geometry) -> Result<Self> {
        let case = &geometry.inputs.case;
        let body = case.namelist("BODY").expect("BODY schema");
        let fl = case.namelist("FLTCON").expect("FLTCON schema");
        let syn = case.namelist("SYNTHS").expect("SYNTHS schema");
        let n = geometry
            .body
            .as_ref()
            .ok_or_else(|| invalid("no body"))?
            .stations
            .x
            .len();
        let nalpha = ((fl.values[1] + 0.5) as i32).clamp(0, 20) as usize;
        let array = |name| &body.get(name).expect("BODY schema")[..n];
        let reference = &geometry.reference;
        Ok(Inputs {
            x: array("X"),
            s: array("S"),
            p: array("P"),
            r: array("R"),
            zu: array("ZU"),
            zl: array("ZL"),
            bnose: body.values[121],
            btail: body.values[122],
            bln: body.values[123],
            bla: body.values[124],
            ds: body.values[125],
            method: body.values[127],
            ellip: body.values[128],
            alphas: &fl.values[22..22 + nalpha],
            sref: reference
                .area
                .ok_or_else(|| invalid("SREF is required for body aerodynamics"))?,
            cbarr: reference
                .chord
                .ok_or_else(|| invalid("CBARR is required for body aerodynamics"))?,
            // The main program defaults BLREF to 2*SSPN, i.e. 2*UNUSED with no wing.
            blref: reference.span.unwrap_or(2.0 * UNUSED),
            roughness: reference.roughness,
            xcg: syn.values[0],
        })
    }
}

fn value(axis: &[f32], values: &[f32], at: f32, l: Limits) -> Result<f32> {
    Ok(interp::tbfunx(axis, values, at, l)?.value)
}
fn last(x: &[f32], y: &[f32]) -> Result<f32> {
    Ok(*trapz(x, y)?.last().expect("nonempty"))
}
fn invalid(message: &str) -> AeroError {
    AeroError::Invalid(message.into())
}

/// GETMAX: abscissa, value and index of the first largest value.
fn getmax(x: &[f32], y: &[f32]) -> (f32, f32, usize) {
    let mut best = 0;
    for i in 1..y.len() {
        if y[i] > y[best] {
            best = i;
        }
    }
    (x[best], y[best], best)
}

/// EQSPC1: S and dS/dX at NE equally spaced X; a single point is repeated
/// with zero slope.
fn eqspc1(x: &[f32], s: &[f32], ne: usize) -> Result<(Vec<f32>, Vec<f32>, Vec<f32>)> {
    let np = x.len();
    if np == 1 {
        return Ok((vec![x[0]; ne], vec![s[0]; ne], vec![0.0; ne]));
    }
    let step = (x[np - 1] - x[0]) / (ne - 1) as f32;
    let mut xe = vec![0.0; ne];
    let mut se = vec![0.0; ne];
    xe[0] = x[0];
    se[0] = s[0];
    xe[ne - 1] = x[np - 1];
    se[ne - 1] = s[np - 1];
    for i in 1..ne - 1 {
        xe[i] = xe[i - 1] + step;
        se[i] = interp::interx(&[x], s, &[xe[i]], &[HOLD])?.value;
    }
    let mut slope = vec![0.0; ne];
    for (i, d) in slope.iter_mut().enumerate() {
        *d = interp::tbfunx(&xe, &se, xe[i], HOLD)?.derivative;
    }
    Ok((xe, se, slope))
}

/// FIG26: turbulent skin friction from Reynolds number and Mach, a quartic
/// in log10(RN) per tabulated Mach, linearly blended between Mach curves
/// unless within 0.02 of the lower one.
pub fn fig26(rn: f32, mach: f32) -> f32 {
    const A: [f32; 9] = [
        4.12963E-6, 3.92725E-6, 4.55853E-6, 4.49735E-6, 4.11442E-6, 4.51587E-6, 4.61971E-6,
        4.53836E-6, 3.86772E-6,
    ];
    const B: [f32; 9] = [
        -1.36204E-4,
        -1.3037E-4,
        -1.48715E-4,
        -1.47407E-4,
        -1.36505E-4,
        -1.47222E-4,
        -1.48773E-4,
        -1.44676E-4,
        -1.23287E-4,
    ];
    const C: [f32; 9] = [
        1.7162E-3, 1.65388E-3, 1.85005E-3, 1.83955E-3, 1.72383E-3, 1.8252E-3, 1.81973E-3,
        1.74944E-3, 1.49335E-3,
    ];
    const D: [f32; 9] = [
        -9.88935E-3,
        -9.59519E-3,
        -1.0503E-2,
        -1.04587E-2,
        -9.91294E-3,
        -1.02911E-2,
        -1.01084E-2,
        -9.59421E-3,
        -8.22227E-3,
    ];
    const E: [f32; 9] = [
        2.23641E-2, 2.18366E-2, 2.33437E-2, 2.32398E-2, 2.22626E-2, 2.2622E-2, 2.18584E-2,
        2.04571E-2, 1.76472E-2,
    ];
    const AMACH: [f32; 9] = [0.0, 0.3, 0.7, 0.9, 1.0, 1.5, 2.0, 2.5, 3.0];
    let x = rn.log10();
    let curve = |l: usize| x * (E[l] + x * (D[l] + x * (C[l] + x * (B[l] + x * A[l]))));
    // Mach below 0 or from 3.0 up uses the last curve unblended.
    let Some(m) = (0..8).find(|&m| mach >= AMACH[m] && mach < AMACH[m + 1]) else {
        return curve(8);
    };
    let ym = curve(m);
    if (mach - AMACH[m]).abs() <= 0.02 {
        return ym;
    }
    let n = m + 1;
    ym + ((mach - AMACH[m]) / (AMACH[n] - AMACH[m])) * (curve(n) - ym)
}

/// Zero-lift drag shared by BODYRT and BODOPT: FIG 4.1.5.1-27 cutoff
/// Reynolds number, FIG26 skin friction, then friction + base drag.
struct Drag {
    length_reynolds: f32,
    cutoff_reynolds: f32,
    skin_friction: f32,
    wetted_area: f32,
    fineness_ratio: f32,
    friction: f32,
    base: f32,
    total: f32,
}

/// `b1` is the original's B(1), normally the Mach number (see
/// [`transonic`] for the exception), and sets the cutoff Reynolds number
/// (FIG 4.1.5.1-27). `fig26_mach` is B(1), except 0.6 for BODYRT at
/// transonic points.
fn zero_lift_drag(
    inp: &Inputs,
    point: &FlightPoint,
    b1: f32,
    fig26_mach: f32,
    wetted_area: f32,
    maximum_area: f32,
    base_area: f32,
) -> Result<Drag> {
    let length = inp.x[inp.x.len() - 1];
    let bd55 = 12.0 * length / inp.roughness;
    let length_reynolds = point.reynolds * length;
    let cept = value(&X27M, &X27I, b1, HOLD)?;
    let cutoff_reynolds = powr(bd55, 1.0482) * powr(10.0, cept);
    let ruse = if cutoff_reynolds < length_reynolds {
        cutoff_reynolds
    } else {
        length_reynolds
    };
    let skin_friction = fig26(ruse, fig26_mach);
    let base_diameter = (base_area * 4. / PI).sqrt();
    let maximum_diameter = (maximum_area * 4. / PI).sqrt();
    let fineness_ratio = length / maximum_diameter;
    let f = fineness_ratio;
    let friction =
        skin_friction * (1. + 60. / (f * f * f) + 0.0025 * f) * wetted_area / maximum_area;
    let q = base_diameter / maximum_diameter;
    let base = 0.029 * (q * q * q / friction.sqrt() * maximum_area / inp.sref);
    let friction = friction * maximum_area / inp.sref;
    let drag = Drag {
        length_reynolds,
        cutoff_reynolds,
        skin_friction,
        wetted_area,
        fineness_ratio,
        friction,
        base,
        total: friction + base,
    };
    finite(&[
        drag.cutoff_reynolds,
        drag.skin_friction,
        drag.fineness_ratio,
        drag.total,
    ])?;
    Ok(drag)
}

/// Per-alpha results of one method, before the overlay's derivative pass.
struct Coefficients {
    cd: Vec<f32>,
    cl: Vec<f32>,
    cm: Vec<f32>,
    cla: f32,
    cma: f32,
}

/// BODYRT: slender-body potential lift plus viscous crossflow. At transonic
/// points it stops after the drag (`IF(TRANSN)RETURN`): the curves and
/// crossflow drag stay empty and `eta` is `None`.
struct Bodyrt {
    coefficients: Coefficients,
    drag: Drag,
    maximum_area: f32,
    base_area: f32,
    apparent_mass_factor: f32,
    eta: Option<f32>,
    crossflow_drag: Vec<f32>,
}

fn bodyrt(inp: &Inputs, point: &FlightPoint, b1: f32) -> Result<Bodyrt> {
    let transonic = point.regime == Regime::Transonic;
    let (x, s) = (inp.x, inp.s);
    let n = x.len();
    let length = x[n - 1];
    // Body alone: the "nose" is the whole body (BD(5) = LNOSE, BD(4) = SLNOSE).
    let nose_length = x[n - 1];
    let nose_area = value(x, s, nose_length, HOLD)?;
    let (_, maximum_area, _) = getmax(x, s);
    let mut base_area = s[n - 1];
    if base_area <= 0.3 * maximum_area {
        base_area = 0.3 * maximum_area;
    }
    // X1: station of steepest area decrease, or the base if S never falls.
    let x1 = if s.windows(2).all(|w| w[1] >= w[0]) {
        x[n - 1]
    } else {
        let slope = limits(Extrapolation::Quadratic, Extrapolation::Linear);
        let mut negative = Vec::with_capacity(n);
        for &xk in x {
            negative.push(-interp::tbfunx(x, s, xk, slope)?.derivative);
        }
        getmax(x, &negative).0
    };
    // X0, the start of viscous separation, and the area there (FIG 4.2.1.1-20).
    let x0 = 0.378 * length + 0.527 * x1;
    let (tmp1, tmp2, tmp3, tmp5) = if x0 > nose_length {
        (nose_area, nose_area, nose_length, nose_length)
    } else {
        (value(x, s, x0, HOLD)?, maximum_area, length, x0)
    };
    let tmp4 = tmp3 / (tmp2 * 4. / PI).sqrt();
    let k = value(
        &X21120,
        &Y21120,
        tmp4,
        limits(Extrapolation::Quadratic, Extrapolation::Linear),
    )?;
    let cla = 2. * k * tmp1 / (RAD * inp.sref);
    let (xe, pe, _) = eqspc1(x, inp.p, 20)?;
    let wetted_area = last(&xe, &pe)?;
    // Temporarily end the forebody at X0 with area S0 (the original edits
    // X(L)/S(L) in place and restores them after the drag calculation).
    let l = x
        .iter()
        .position(|&v| v >= tmp5)
        .ok_or_else(|| invalid("separation station lies beyond the body"))?;
    let mut xm = x.to_vec();
    let mut sm = s.to_vec();
    xm[l] = tmp5;
    sm[l] = tmp1;
    let il = (1..=l)
        .rfind(|&k| sm[k] - sm[k - 1] != 0.0)
        .ok_or_else(|| invalid("body forebody area never changes"))?
        + 1;
    let (xe, _, dsedx) = eqspc1(&xm[..il], &sm[..il], 20)?;
    let tst: Vec<f32> = dsedx.iter().zip(&xe).map(|(d, x)| d * x).collect();
    let tmp = last(&xe, &tst)?;
    let cnst = 2. * k / (RAD * inp.sref * inp.cbarr);
    let cma = (inp.xcg * cla / inp.cbarr) - cnst * tmp;
    // Afterbody planform (from X0, radius R) for the crossflow terms.
    let (xe, re, _) = eqspc1(&xm[l..], &inp.r[l..], 20)?;
    let planform = last(&xe, &re)?;
    let tst: Vec<f32> = re.iter().zip(&xe).map(|(r, x)| r * x).collect();
    let moment = last(&xe, &tst)?;
    let fig26_mach = if transonic { 0.6 } else { b1 };
    let drag = zero_lift_drag(
        inp,
        point,
        b1,
        fig26_mach,
        wetted_area,
        maximum_area,
        base_area,
    )?;
    let count = inp.alphas.len();
    let mut c = Coefficients {
        cd: Vec::with_capacity(count),
        cl: Vec::with_capacity(count),
        cm: Vec::with_capacity(count),
        cla,
        cma,
    };
    if transonic {
        return Ok(Bodyrt {
            coefficients: c,
            drag,
            maximum_area,
            base_area,
            apparent_mass_factor: k,
            eta: None,
            crossflow_drag: Vec::new(),
        });
    }
    let eta = value(
        &X1217A,
        &Y1217A,
        drag.fineness_ratio,
        limits(Extrapolation::Quadratic, Extrapolation::Quadratic),
    )?;
    let mut crossflow_drag = Vec::with_capacity(count);
    for &alpha in inp.alphas {
        // SETUP1's ALPHAB = alpha + BD(81); INITZE leaves BD(81) = UNUSED for
        // every case, so BODOPT's zero-lift angle never reaches BODYRT.
        let ap = alpha + UNUSED;
        let sina = (ap / RAD).sin();
        let sina2 = sina * sina;
        let potential = cla * ap;
        let cdc = value(&X1217B, &Y1217B, point.mach * sina.abs(), HOLD)?;
        let sgn = if ap < 0. { -1.0 } else { 1.0 };
        let viscous = 2. * sina2 * eta * cdc * planform / inp.sref * sgn;
        // BODYRT stores this normal force in the CL slot BODY(J+20).
        let cn = potential + viscous;
        let cm = cma * ap
            - 2. * sina2 * cdc * eta * (moment - inp.xcg * planform) / (inp.cbarr * inp.sref) * sgn;
        let cd = drag.total + (potential * sina + viscous * sina);
        c.cl.push(cn);
        c.cm.push(cm);
        c.cd.push(cd);
        crossflow_drag.push(cdc);
    }
    Ok(Bodyrt {
        coefficients: c,
        drag,
        maximum_area,
        base_area,
        apparent_mass_factor: k,
        eta: Some(eta),
        crossflow_drag,
    })
}

/// BODYJM: Jorgensen's method replacing BODYRT's CD/CL/CM; reuses BODYRT's
/// boat-tail base area, eta, crossflow drag and (for circular sections)
/// its CLA/CMA as the potential terms. ELLIP is the width/depth ratio.
fn bodyjm(inp: &Inputs, geometry: &Geometry, rt: &Bodyrt) -> Result<Coefficients> {
    let body = geometry.body.as_ref().expect("body geometry");
    let (vb, sp, xc) = (body.volume, body.projected_area, body.projected_centroid);
    let (sref, cbarr, xcg) = (inp.sref, inp.cbarr, inp.xcg);
    let sb = rt.base_area;
    let lb = inp.x[inp.x.len() - 1];
    let c = &rt.coefficients;
    let mut a1 = (vb - sb * (lb - xcg)) / (sref * cbarr);
    let a2 = (sp * (xcg - xc)) / (sref * cbarr);
    let mut a3 = sb;
    let ellip = if inp.ellip <= UNUSED { 1.0 } else { inp.ellip };
    if ellip == 1.0 {
        a1 = c.cma * RAD / 2.0;
        a3 = c.cla * sref * RAD / 2.0;
    }
    let (cnocns, cnocnn) = ellipse_factors(ellip);
    let eta = rt.eta.expect("subsonic BODYRT sets eta");
    let cd0 = rt.drag.total;
    let count = inp.alphas.len();
    let mut out = Coefficients {
        cd: Vec::with_capacity(count),
        cl: Vec::with_capacity(count),
        cm: Vec::with_capacity(count),
        cla: c.cla,
        cma: c.cma,
    };
    for (i, &alpha) in inp.alphas.iter().enumerate() {
        let alp = alpha / RAD;
        let (sa, cs) = (alp.sin(), alp.cos());
        let fa = (2.0 * alp).sin() * (alp / 2.0).cos();
        let cdc = rt.crossflow_drag[i];
        let cn = (a3 * fa * cnocns + sp * eta * cdc * sa * sa.abs() * cnocnn) / sref;
        let cm = a1 * fa * cnocns + a2 * eta * cdc * sa * sa.abs() * cnocnn;
        let ca = cd0 * (cs * cs);
        out.cl.push(cn * cs - ca * sa);
        out.cd.push(ca * cs + cn * sa);
        out.cm.push(cm);
    }
    Ok(out)
}

/// BODOPT results beyond the shared coefficients.
struct Bodopt {
    coefficients: Coefficients,
    drag: Drag,
    maximum_area: f32,
    base_area: f32,
    alpha_zero: f32,
    cm0: f32,
}

/// BODOPT: cambered/asymmetric body from the ZU/ZL camber line, with
/// potential (Kp) and vortex (Kv) normal force from the planform aspect ratio.
fn bodopt(inp: &Inputs, point: &FlightPoint) -> Result<Bodopt> {
    let (x, s, r) = (inp.x, inp.s, inp.r);
    let n = x.len();
    let lref = inp.cbarr;
    let (_, maximum_area, _) = getmax(x, s);
    let mut base_area = s[n - 1];
    if base_area <= 0.3 * maximum_area {
        base_area = 0.3 * maximum_area;
    }
    let stations = |radius: Vec<f32>, perimeter: Vec<f32>| BodyStations {
        x: x.to_vec(),
        radius,
        perimeter,
        area: s.to_vec(),
    };
    let geometry_error = |e| AeroError::Invalid(format!("{e}"));
    let eq = eqspce(&stations(r.to_vec(), inp.p.to_vec()), 20).map_err(geometry_error)?;
    let wetted_area = last(&eq.stations.x, &eq.stations.perimeter)?;
    let drag = zero_lift_drag(
        inp,
        point,
        point.mach,
        point.mach,
        wetted_area,
        maximum_area,
        base_area,
    )?;
    // Camber line relative to the chord joining the end midpoints.
    let z0 = 0.5 * (inp.zu[0] + inp.zl[0]);
    let znx = 0.5 * (inp.zu[n - 1] + inp.zl[n - 1]);
    let dzdx = (znx - z0) / (x[n - 1] - x[0]);
    let mut depth = vec![0.0; n];
    let mut xol = vec![0.0; n];
    let mut zpol = vec![0.0; n];
    for i in 0..n {
        depth[i] = inp.zu[i] - inp.zl[i];
        if depth[i] == 0. && i != 0 {
            depth[i] = UNUSED;
        }
        let zp = inp.zl[i] + 0.5 * depth[i] - (x[i] * dzdx + z0);
        zpol[i] = zp / x[n - 1];
        xol[i] = x[i] / x[n - 1];
    }
    finite(&zpol)?;
    let xovlcg = inp.xcg / x[n - 1];
    let camber = eqspce_signed(&stations(xol.clone(), zpol), 20).map_err(geometry_error)?;
    let (xole, zpole) = (&camber.stations.radius, &camber.stations.perimeter);
    // Thin-body zero-lift angle and moment integrands (zero at both ends).
    let mut alph = [0.0f32; 20];
    let mut cmp = [0.0f32; 20];
    for l in 1..19 {
        let root = (xole[l] - xole[l] * xole[l]).sqrt();
        alph[l] = zpole[l] / ((1.0 - xole[l]) * root);
        cmp[l] = zpole[l] * ((1. - 2. * xole[l]) / root);
    }
    let alzero = last(&xole[..19], &alph[..19])?;
    let alpha_zero = -alzero / PI * RAD;
    let plan = eqspce(&stations(r.to_vec(), xol), 20).map_err(geometry_error)?;
    let (xe, ye) = (&plan.stations.x, &plan.stations.radius);
    let sp = 2.0 * last(xe, ye)?;
    let cmzp = last(&plan.stations.perimeter, &cmp)?;
    let cm0 = 2. * cmzp * (sp / inp.sref) * (x[n - 1] / lref);
    // Body aspect ratio and the station where the half-width reaches 96%.
    let (_, ymax, imax) = getmax(x, r);
    let arb = (4.0 * ymax * ymax) / sp;
    let y95 = 0.96 * ymax;
    if imax == 0 {
        return Err(invalid(
            "BODOPT needs the maximum half-width aft of the nose",
        ));
    }
    let im = imax - 1;
    let x95 = x[im] + (x[imax] - x[im]) * (y95 - r[im]) / (r[imax] - r[im]);
    let akp = value(
        &XBA,
        &YBA1,
        arb,
        limits(Extrapolation::Hold, Extrapolation::Quadratic),
    )?;
    let akv = value(
        &XBA,
        &YBA2,
        arb,
        limits(Extrapolation::Hold, Extrapolation::Quadratic),
    )?;
    let smax = s[n - 1];
    let deff = (4.0 * smax / PI).sqrt();
    let (ratio, fr) = if deff == 0. {
        (3.0, 10.0)
    } else {
        (2.0 * r[n - 1] / depth[n - 1], x[n - 1] / deff)
    };
    // Planform area forward of X95, with the outline ending at (X95, Y95).
    let n95 = x
        .iter()
        .position(|&xi| x95 <= xi)
        .ok_or_else(|| invalid("BODOPT 96% half-width station lies beyond the body"))?
        + 1;
    let mut tx = x[..n95].to_vec();
    let mut ty = r[..n95].to_vec();
    tx[n95 - 1] = x95;
    ty[n95 - 1] = y95;
    let ones = vec![1.0; n95];
    let front = eqspce(
        &BodyStations {
            x: tx,
            radius: ty,
            perimeter: ones.clone(),
            area: ones,
        },
        20,
    )
    .map_err(geometry_error)?;
    let a95max = 2.0 * last(&front.stations.x, &front.stations.radius)?;
    let quadratic = limits(Extrapolation::Quadratic, Extrapolation::Quadratic);
    let alphv = interp::tlin(
        &[&X1BA3, &X2BA3],
        &YBA3,
        &[ratio, fr],
        &[quadratic, quadratic],
    )?
    .value;
    // Centre of the potential (CNP) and vortex (planform centroid) loads.
    let expon = (2.0 * y95 * x95) / a95max - 1.00;
    let xolcnp = 2.0 * expon / (2.0 * expon + 1.0);
    let delxol = xovlcg - xolcnp * (x95 / x[n - 1]);
    let mut dela = trapz(xe, ye)?;
    for j in 0..19 {
        dela[j] = dela[j + 1] - dela[j];
    }
    let mut sum = 0.0;
    for k in 0..19 {
        sum += 2.0 * dela[k] * (xe[k] + (xe[k + 1] - xe[k]) / 2.0);
    }
    let xovlca = (sum / sp) / x[n - 1];
    let count = inp.alphas.len();
    let mut c = Coefficients {
        cd: Vec::with_capacity(count),
        cl: Vec::with_capacity(count),
        cm: Vec::with_capacity(count),
        cla: (akp * sp) / (RAD * inp.sref),
        cma: akp * delxol * (sp / inp.sref) * (x[n - 1] / lref) / RAD,
    };
    for &alpha in inp.alphas {
        let angr = (alpha - alpha_zero) / RAD;
        let (sa, ca) = (angr.sin(), angr.cos());
        let cnp = akp * sa * ca;
        let cnv = if alpha <= alphv {
            0.0
        } else {
            let v = ((alpha - alphv) / RAD).sin();
            akv * (v * v)
        };
        let cn = (cnp + cnv) * (sp / inp.sref);
        let cd = cn * sa + drag.total;
        let cm = cm0
            + cnp * delxol * (sp / inp.sref) * (x[n - 1] / lref)
            + cnv * (xovlcg - xovlca) * (sp / inp.sref) * (x[n - 1] / lref);
        let a = alpha / RAD;
        c.cl.push((cn - cd * a.sin()) / a.cos());
        c.cd.push(cd);
        c.cm.push(cm);
    }
    Ok(Bodopt {
        coefficients: c,
        drag,
        maximum_area,
        base_area,
        alpha_zero,
        cm0,
    })
}

/// MAIN01's body branch for one subsonic point: choose the method like the
/// original, then apply M04O04/M06O06's alpha-derivative and axis pass.
/// `experimental` is the point's EXPR data (EXPDAT), if any.
pub fn subsonic(
    geometry: &Geometry,
    point: &FlightPoint,
    experimental: Option<&Namelist>,
) -> Result<BodyAero> {
    let inp = Inputs::new(geometry)?;
    let nalpha = inp.alphas.len();
    let n = inp.x.len();
    let asymmetric = !(inp.zl[0] == UNUSED || inp.method > 1.5);
    let (method, mut c, drag, maximum_area, base_area, extra) = if asymmetric {
        let o = bodopt(&inp, point)?;
        let extra = (None, None, Vec::new(), Some(o.alpha_zero), Some(o.cm0));
        (
            BodyMethod::Asymmetric,
            o.coefficients,
            o.drag,
            o.maximum_area,
            o.base_area,
            extra,
        )
    } else {
        let rt = bodyrt(&inp, point, point.mach)?;
        let (method, c) = if inp.method > 1.5 {
            (BodyMethod::Jorgensen, bodyjm(&inp, geometry, &rt)?)
        } else {
            (BodyMethod::Axisymmetric, rt.coefficients)
        };
        let extra = (
            Some(rt.apparent_mass_factor),
            rt.eta,
            rt.crossflow_drag,
            None,
            None,
        );
        (method, c, rt.drag, rt.maximum_area, rt.base_area, extra)
    };
    // M04O04/M06O06 after EXSUBT: measured CDB/CLB/CMB/CLAB/CMAB replace
    // computed values; KBODY (any first entry given) then takes CM0 at
    // CL = 0 and re-derives every slope, including the first, from the curves.
    let mut cla = vec![c.cla; nalpha];
    let mut cma = vec![c.cma; nalpha];
    let mut kbody = false;
    if let Some(data) = experimental {
        let v = &data.values;
        for (slot, target) in [
            (0, &mut c.cd),
            (20, &mut c.cl),
            (40, &mut c.cm),
            (60, &mut cla),
            (80, &mut cma),
        ] {
            kbody |= v[slot] != UNUSED;
            for (j, t) in target.iter_mut().enumerate() {
                if v[slot + j] != UNUSED {
                    *t = v[slot + j];
                }
            }
        }
    }
    let (apparent_mass_factor, eta, crossflow_drag, alpha_zero, mut cm0) = extra;
    if kbody {
        let linear = limits(Extrapolation::Linear, Extrapolation::Linear);
        cm0 = Some(interp::tbfunx_unordered(&c.cl, &c.cm, 0.0, linear)?.value);
    }
    // Slopes from the CL/CM curves (without KBODY the first alpha keeps the
    // method's value), lateral derivatives, then body-axis CN/CA.
    let bfact = inp.cbarr / inp.blref;
    let (mut cn, mut ca) = (Vec::with_capacity(nalpha), Vec::with_capacity(nalpha));
    for j in 0..nalpha {
        if j > 0 || kbody {
            let at = inp.alphas[j];
            cla[j] = interp::tbfunx_unordered(inp.alphas, &c.cl, at, HOLD)?.derivative;
            cma[j] = interp::tbfunx_unordered(inp.alphas, &c.cm, at, HOLD)?.derivative;
        }
        let a = inp.alphas[j] / RAD;
        let (cosa, sina) = (a.cos(), a.sin());
        cn.push(c.cl[j] * cosa + c.cd[j] * sina);
        ca.push(c.cd[j] * cosa - c.cl[j] * sina);
    }
    let cyb: Vec<f32> = cla.iter().map(|v| -v).collect();
    let cnb: Vec<f32> = cma.iter().map(|v| -bfact * v).collect();
    for v in [&c.cd, &c.cl, &c.cm, &cn, &ca, &cla, &cma, &cyb, &cnb] {
        finite(v)?;
    }
    Ok(BodyAero {
        method,
        cd: std::mem::take(&mut c.cd),
        cl: std::mem::take(&mut c.cl),
        cm: std::mem::take(&mut c.cm),
        cn,
        ca,
        cla,
        cma,
        cyb,
        cnb,
        clb: vec![0.0; nalpha],
        slender: Some(SlenderBody::new(
            inp.x[n - 1],
            maximum_area,
            base_area,
            &drag,
            (apparent_mass_factor, eta, crossflow_drag, alpha_zero, cm0),
        )),
        supersonic: None,
        hypersonic: None,
    })
}

impl SlenderBody {
    /// Gather the drag build-up plus the method values `(k, eta, crossflow
    /// drag, alpha zero, CM0)`.
    #[allow(clippy::type_complexity)]
    fn new(
        length: f32,
        maximum_area: f32,
        base_area: f32,
        drag: &Drag,
        extra: (Option<f32>, Option<f32>, Vec<f32>, Option<f32>, Option<f32>),
    ) -> Self {
        let (apparent_mass_factor, eta, crossflow_drag, alpha_zero, cm0) = extra;
        SlenderBody {
            length,
            maximum_area,
            base_area,
            fineness_ratio: drag.fineness_ratio,
            length_reynolds: drag.length_reynolds,
            cutoff_reynolds: drag.cutoff_reynolds,
            skin_friction: drag.skin_friction,
            wetted_area: drag.wetted_area,
            friction_drag: drag.friction,
            base_drag: drag.base,
            zero_lift_drag: drag.total,
            apparent_mass_factor,
            eta,
            crossflow_drag,
            alpha_zero,
            cm0,
        }
    }
}

// The BODY(1..200) common block as the overlays leave it: CD 0.., CL 20..,
// CM 40.., CN 60.., CA 80.., CLA 100.., CMA 120.., CYB 140.., CNB 160..,
// CLB 180.. (0-based). The transonic/supersonic drivers below replay the
// original overlay sequence on this array so that EXPR substitutions and
// the UNUSED/-UNUSED markers end up exactly where the original puts them.
const SLOTS: usize = 200;
const CD: usize = 0;
const CL: usize = 20;
const CM: usize = 40;
const CN: usize = 60;
const CA: usize = 80;
const CLA: usize = 100;
const CMA: usize = 120;
const CYB: usize = 140;
const CNB: usize = 160;
const CLB: usize = 180;

/// EXSUBT for the body: given EXPR values replace BODY's CD, CL, CM, CLA and
/// CMA entries (EXPR slots 0/20/40/60/80). Returns KBODY, true when any of
/// the five first entries is given. A no-op without EXPR data.
fn exsubt(experimental: Option<&Namelist>, b: &mut [f32], nalpha: usize) -> bool {
    let Some(data) = experimental else {
        return false;
    };
    let v = &data.values;
    let mut kbody = false;
    for (slot, base) in [(0, CD), (20, CL), (40, CM), (60, CLA), (80, CMA)] {
        kbody |= v[slot] != UNUSED;
        for j in 0..nalpha {
            if v[slot + j] != UNUSED {
                b[base + j] = v[slot + j];
            }
        }
    }
    kbody
}

/// TBFUNX slope (hold extrapolation) of the curve at BODY(base..) at alpha j.
fn slope(alphas: &[f32], b: &[f32], base: usize, j: usize) -> Result<f32> {
    let curve = &b[base..base + alphas.len()];
    Ok(interp::tbfunx_unordered(alphas, curve, alphas[j], HOLD)?.derivative)
}

/// Body-axis CN/CA from the stability-axis CL/CD in the slots, alpha j.
fn axis_pass(alphas: &[f32], b: &mut [f32], j: usize) {
    let a = alphas[j] / RAD;
    let (ca, sa) = (a.cos(), a.sin());
    b[CN + j] = b[CL + j] * ca + b[CD + j] * sa;
    b[CA + j] = b[CD + j] * ca - b[CL + j] * sa;
}

/// Store per-alpha method curves into their BODY slots.
fn store(b: &mut [f32], curves: &supersonic::Curves) {
    for (base, curve) in [
        (CD, &curves.cd),
        (CL, &curves.cl),
        (CM, &curves.cm),
        (CN, &curves.cn),
        (CA, &curves.ca),
    ] {
        b[base..base + curve.len()].copy_from_slice(curve);
    }
}

/// Unpack the BODY slots into a [`BodyAero`].
fn from_slots(b: &[f32], nalpha: usize, method: BodyMethod) -> Result<BodyAero> {
    let take = |base: usize| -> Result<Vec<f32>> {
        let v = b[base..base + nalpha].to_vec();
        finite(&v)?;
        Ok(v)
    };
    Ok(BodyAero {
        method,
        cd: take(CD)?,
        cl: take(CL)?,
        cm: take(CM)?,
        cn: take(CN)?,
        ca: take(CA)?,
        cla: take(CLA)?,
        cma: take(CMA)?,
        cyb: take(CYB)?,
        cnb: take(CNB)?,
        clb: take(CLB)?,
        slender: None,
        supersonic: None,
        hypersonic: None,
    })
}

/// MAIN03's body path for one transonic point. BODYRT gives only the
/// alpha(1) slopes and drag (its curves need EXPR data), SYPBOD runs in
/// transonic mode for SBD alone, then M24O30 marks every alpha after the
/// first as not computed (-UNUSED) and CNCA re-derives what EXPR allows.
///
/// `b1` is the original's B(1) as BODYRT sees it: the Mach number, except
/// at the case's first transonic point without lifting surfaces, where
/// MAIN03's DOL2 pre-pass (SETUP2) leaves B(1) = TRAH(6) = UNUSED.
pub fn transonic(
    geometry: &Geometry,
    point: &FlightPoint,
    experimental: Option<&Namelist>,
    b1: f32,
) -> Result<BodyAero> {
    let inp = Inputs::new(geometry)?;
    let (alphas, na) = (inp.alphas, inp.alphas.len());
    let mut b = vec![UNUSED; SLOTS];
    exsubt(experimental, &mut b, na);
    // M06O06: BODYRT (BODYJM is subsonic only), EXSUBT, slope/axis pass.
    let rt = bodyrt(&inp, point, b1)?;
    b[CLA] = rt.coefficients.cla;
    b[CMA] = rt.coefficients.cma;
    let kbody = exsubt(experimental, &mut b, na);
    let bfact = inp.cbarr / inp.blref;
    for j in 0..na {
        if j > 0 || kbody {
            b[CLA + j] = slope(alphas, &b, CL, j)?;
            b[CMA + j] = slope(alphas, &b, CM, j)?;
        }
        b[CYB + j] = -b[CLA + j];
        b[CNB + j] = -bfact * b[CMA + j];
        b[CLB + j] = 0.0;
        axis_pass(alphas, &mut b, j);
    }
    // M19O23: transonic SYPBOD fills SBD only, then EXSUBT and return. The
    // original carries on with NaN/Inf SBD (e.g. BLN unset gives a zero nose
    // diameter); nothing reads SBD for a lone body, so rudatcom reports no
    // SYPBOD values instead of failing the point.
    let sbd = match supersonic::sypbod(&inp, point) {
        Ok((sbd, _)) => Some(sbd),
        Err(AeroError::Numerical(_)) => None,
        Err(e) => return Err(e),
    };
    exsubt(experimental, &mut b, na);
    // M24O30.
    exsubt(experimental, &mut b, na);
    b[CYB] = -b[CLA];
    b[CNB] = -b[CMA] * inp.cbarr / inp.blref;
    for j in 0..na {
        b[CN + j] = UNUSED;
        b[CA + j] = UNUSED;
        if j > 0 {
            for base in [CL, CM, CN, CA, CLA, CMA, CYB, CNB] {
                b[base + j] = -UNUSED;
            }
        }
        b[CLB + j] = 0.0;
    }
    exsubt(experimental, &mut b, na);
    // CNCA (via M35O43/SECLEV): only entries with a known CL (and CD).
    for j in 0..na {
        let known_cl = b[CL + j].abs() != UNUSED;
        if known_cl && j >= 1 {
            b[CLA + j] = slope(alphas, &b, CL, j)?;
        }
        if known_cl && b[CD + j].abs() != UNUSED {
            axis_pass(alphas, &mut b, j);
        }
    }
    let mut out = from_slots(&b, na, BodyMethod::Transonic)?;
    out.slender = Some(SlenderBody::new(
        inp.x[inp.x.len() - 1],
        rt.maximum_area,
        rt.base_area,
        &rt.drag,
        (Some(rt.apparent_mass_factor), None, Vec::new(), None, None),
    ));
    out.supersonic = sbd;
    Ok(out)
}

/// MAIN04's body path for one supersonic point: M26O32 (HYPBOD) for FLTCON
/// HYPERS with METHOD = 1, else M19O23 (SYPBOD). Both form the slopes from
/// the alpha curves; only M19O23 applies EXPR before doing so and
/// recomputes CN/CA.
pub fn supersonic(
    geometry: &Geometry,
    point: &FlightPoint,
    experimental: Option<&Namelist>,
    hypersonic: bool,
) -> Result<BodyAero> {
    let inp = Inputs::new(geometry)?;
    let (alphas, na) = (inp.alphas, inp.alphas.len());
    let bfact = inp.cbarr / inp.blref;
    let mut b = vec![UNUSED; SLOTS];
    exsubt(experimental, &mut b, na);
    let (method, sbd, hbd) = if hypersonic {
        let (hbd, curves) = supersonic::hypbod(&inp, point)?;
        store(&mut b, &curves);
        (BodyMethod::Hypersonic, None, Some(hbd))
    } else {
        let (sbd, curves) = supersonic::sypbod(&inp, point)?;
        store(&mut b, &curves);
        exsubt(experimental, &mut b, na);
        (BodyMethod::Supersonic, Some(sbd), None)
    };
    for j in 0..na {
        b[CLA + j] = slope(alphas, &b, CL, j)?;
        b[CMA + j] = slope(alphas, &b, CM, j)?;
        if !hypersonic {
            axis_pass(alphas, &mut b, j);
        }
        b[CYB + j] = -b[CLA + j];
        b[CNB + j] = -bfact * b[CMA + j];
        b[CLB + j] = 0.0;
    }
    // MAIN04's closing EXSUBT.
    exsubt(experimental, &mut b, na);
    let mut out = from_slots(&b, na, method)?;
    out.supersonic = sbd;
    out.hypersonic = hbd;
    Ok(out)
}
