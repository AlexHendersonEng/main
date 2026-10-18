//! Wing-alone aerodynamics. Checkpoint 7d ports the subsonic path of MAIN01:
//! M15O17 (CALCA0, WTLIFT, LIFTCF, CLMXBS), M03O03 (CDRAG, FIG53A), M31O37
//! (CMALPH, FWDXAC, CACALC and the slope pass), M29O35's wing part and
//! M17O21's SUBLAT, plus the main program's once-per-case CLMCH0 pre-pass.
//!
//! State model: the original keeps the wing work arrays A(195)/B(49)
//! (COMMON /WINGD/) and WINGIN alive for the whole case, and several
//! routines write to them (CMALPH, CDRAG, M29O35's defaults), so
//! [`WingCase`] holds 1-based copies that persist across flight points.
//! WING, C, D and STB are reset to UNUSED per point, as INITZ1 does.
//! Index 0 of every array is unused so subscripts read like the Fortran.

mod data;
mod drag;
mod lateral;
mod lift;
mod moment;

use super::{AeroError, Result};
use crate::{
    flight::{FlightPoint, Regime},
    geometry::{surface, Geometry, GeometryError},
    input::{Namelist, UNUSED},
    interp::{self, Extrapolation, Limits},
    math::finite,
};

/// WINGIN(15) planform types (the original's Hollerith WTYPE names).
const STRA: u8 = 1;
const DOUB: u8 = 2;
const CRAN: u8 = 3;
const CURV: u8 = 4;

/// Wing-alone results for one flight point: the WING(1..200) blocks, one
/// value per angle of attack, plus the work arrays for traceability.
#[derive(Debug, Clone)]
pub struct WingAero {
    pub cd: Vec<f32>,
    pub cl: Vec<f32>,
    /// 2*UNUSED marks moments the original declines to report.
    pub cm: Vec<f32>,
    pub cn: Vec<f32>,
    pub ca: Vec<f32>,
    pub cla: Vec<f32>,
    pub cma: Vec<f32>,
    /// Lateral derivatives (per degree, body axes at the reference span).
    pub cyb: Vec<f32>,
    pub cnb: Vec<f32>,
    pub clb: Vec<f32>,
    /// Method messages the original prints (e.g. "NO CLALPHA COMPUTATION").
    pub messages: Vec<String>,
    /// COMMON-block work arrays after the point, 0-based copies of
    /// A(1..195), B(1..49), C(1..51), D(1..55), STB(1..135), WINGIN(1..91).
    pub work: WingWork,
}

#[derive(Debug, Clone)]
pub struct WingWork {
    pub a: Vec<f32>,
    pub b: Vec<f32>,
    pub c: Vec<f32>,
    pub d: Vec<f32>,
    pub stb: Vec<f32>,
    pub wingin: Vec<f32>,
}

/// Per-case wing state (COMMON /WINGD/, /WINGI/, /IWING/, /WHAERO/ C and
/// D, /SBETA/ STB) plus the case constants the routines read.
pub struct WingCase {
    a: [f32; 196],
    b: [f32; 50],
    ain: [f32; 92],
    wing: [f32; 401],
    c: [f32; 52],
    d: [f32; 56],
    stb: [f32; 136],
    kind: u8,
    /// WGSCHR CAMBER (WINGIN(64) as a LOGICAL: true only when 1.0).
    camber: bool,
    /// BO: a body is present (CDRAG's wing-body lift correction).
    body: bool,
    /// FLTCON angles of attack (FLC(23..)), without the wing incidence.
    alphas: Vec<f32>,
    sref: f32,
    cbarr: f32,
    blref: f32,
    roughness: f32,
    /// FLC(I+2) and the 1-based Mach index I of the current point.
    mach: f32,
    i: usize,
    messages: Vec<String>,
}

fn invalid(message: &str) -> AeroError {
    AeroError::Invalid(message.into())
}

/// Map an original TLIN/TBFUNX extrapolation flag to [`Extrapolation`].
fn lim(flag: i32) -> Extrapolation {
    match flag {
        f if f < 0 => Extrapolation::HoldSilent,
        0 => Extrapolation::Hold,
        1 => Extrapolation::Linear,
        _ => Extrapolation::Quadratic,
    }
}

/// TLINEX/TLIN3X/TLIN4X with axes and values in Fortran argument order and
/// flags as written in the call: all lower flags, then all upper flags.
fn tl(axes: &[&[f32]], values: &[f32], at: &[f32], flags: &[i32]) -> Result<f32> {
    let n = axes.len();
    let limits: Vec<Limits> = (0..n)
        .map(|k| Limits {
            lower: lim(flags[k]),
            upper: lim(flags[n + k]),
        })
        .collect();
    Ok(interp::tlin(axes, values, at, &limits)?.value)
}

/// TBFUNX value with the call's (LEXL, LEXU) flags. The unordered variant
/// is the original algorithm without rudatcom's extra axis check.
fn tb(axis: &[f32], values: &[f32], at: f32, lower: i32, upper: i32) -> Result<f32> {
    let limits = Limits {
        lower: lim(lower),
        upper: lim(upper),
    };
    Ok(interp::tbfunx_unordered(axis, values, at, limits)?.value)
}

/// ANGLES(1, X) on a 6-slot block (degrees, radians, sin, cos, tan, last).
fn angles(block: &mut [f32]) -> Result<()> {
    surface::angles(1, block).map_err(|e| match e {
        GeometryError::Numerical(m) => AeroError::Numerical(m),
        other => AeroError::Invalid(other.to_string()),
    })
}

impl WingCase {
    /// Build the case state (WINGIN, A after M02O02, B after SETUP1) and run
    /// the main program's CLMCH0 pre-pass when the case has subsonic or
    /// transonic Mach numbers. `None` without a wing.
    pub fn new(geometry: &Geometry, points: &[FlightPoint]) -> Result<Option<Self>> {
        let Some(surface) = &geometry.wing else {
            return Ok(None);
        };
        let case = &geometry.inputs.case;
        let sc = case.namelist("WGSCHR").expect("WGSCHR schema");
        let fl = case.namelist("FLTCON").expect("FLTCON schema");
        let syn = case.namelist("SYNTHS").expect("SYNTHS schema");
        let p = &surface.planform;
        let kind = p.kind;
        let mut ain = [UNUSED; 92];
        // WINGIN(1..15) after WTGEOM (converted units, CHRDBP = tip chord
        // for a single panel). XNAM4 never stores SWAFP: WINGIN(10) is
        // WGSCHR's ALPHAO, so it is filled from the WGSCHR block below.
        let planform = [
            p.tip_chord,
            p.outboard_semispan,
            p.exposed_semispan,
            p.semispan,
            p.break_chord,
            p.root_chord,
            p.inboard_sweep,
            p.outboard_sweep,
            p.chord_station,
            UNUSED,
            p.twist,
            p.dihedral_break,
            p.inboard_dihedral,
            p.outboard_dihedral,
            f32::from(kind),
        ];
        ain[1..16].copy_from_slice(&planform);
        // WINGIN(10) and WINGIN(16..91) are the WGSCHR block in namelist
        // location order.
        ain[10] = sc.values[9];
        ain[16..92].copy_from_slice(&sc.values[15..91]);
        // CAMBER is LOGICAL in WINGIN(64): unset reads as .FALSE. (0).
        let camber = ain[64] == 1.0;
        ain[64] = if camber { 1.0 } else { 0.0 };
        let mut a = [0.0; 196];
        a[1..].copy_from_slice(&surface.coefficients[..195]);
        let nalpha = ((fl.values[1] + 0.5) as i32).clamp(0, 20) as usize;
        let alphas = fl.values[22..22 + nalpha].to_vec();
        // SETUP1: B(J+22) = alpha + ALIW (UNUSED incidence adds 1e-30).
        let mut b = [UNUSED; 50];
        for (j, &alpha) in alphas.iter().enumerate() {
            b[j + 23] = alpha + syn.values[3];
        }
        let reference = &geometry.reference;
        let mut state = WingCase {
            a,
            b,
            ain,
            wing: [UNUSED; 401],
            c: [UNUSED; 52],
            d: [UNUSED; 56],
            stb: [UNUSED; 136],
            kind,
            camber,
            body: geometry.body.is_some(),
            alphas,
            sref: reference
                .area
                .ok_or_else(|| invalid("SREF is required for wing aerodynamics"))?,
            cbarr: reference
                .chord
                .ok_or_else(|| invalid("CBARR is required for wing aerodynamics"))?,
            blref: reference.span.unwrap_or(2.0 * UNUSED),
            roughness: reference.roughness,
            mach: UNUSED,
            i: 1,
            messages: Vec::new(),
        };
        let regime = |r| points.iter().any(|p| p.regime == r);
        if regime(Regime::Subsonic) || regime(Regime::Transonic) {
            state.clmch0(regime(Regime::Transonic))?;
        }
        Ok(Some(state))
    }

    /// CLMCH0: run M15O17 at Mach 0 (B(1) = 0, beta = 1) with the section
    /// lift-curve slope (CLAMO when the case has transonic Machs; the
    /// original's TRANSN can also carry over from the previous case's last
    /// point, which is not modelled) and keep the incompressible CL curve
    /// in B(3..22), CLalpha in B(48) and alpha(CLmax)/alpha0 in A(127)/A(126).
    fn clmch0(&mut self, transonic: bool) -> Result<()> {
        self.b[1] = 0.0;
        self.a[131] = if transonic {
            self.ain[69]
        } else {
            self.ain[21]
        };
        self.a[132] = self.ain[68];
        self.b[2] = 1.0;
        self.wing[1..=20].fill(0.0);
        self.i = 1;
        self.m15o17()?;
        self.b[48] = self.wing[101];
        for j in 1..=self.alphas.len() {
            self.b[j + 2] = self.wing[j + 20];
        }
        self.a[127] = self.b[43];
        self.a[126] = self.b[49];
        Ok(())
    }

    /// M15O17: zero-lift angle, lift-curve slope/CLmax and the CL curve.
    fn m15o17(&mut self) -> Result<()> {
        self.calca0()?;
        self.wtlift()?;
        self.liftcf()
    }

    /// EXSUBT for the wing: given EXPR values replace WING's CD, CL, CM,
    /// CLA and CMA entries (EXPR slots 100/120/140/160/180). Returns KWING,
    /// true when any of the five first entries is given.
    fn exsubt(&mut self, experimental: Option<&Namelist>) -> bool {
        let Some(data) = experimental else {
            return false;
        };
        let v = &data.values;
        let mut kwing = false;
        for (slot, base) in [(100, 1), (120, 21), (140, 41), (160, 101), (180, 121)] {
            kwing |= v[slot] != UNUSED;
            for j in 0..self.alphas.len() {
                if v[slot + j] != UNUSED {
                    self.wing[base + j] = v[slot + j];
                }
            }
        }
        kwing
    }

    /// MAIN01's wing sequence for one subsonic point (MAIN00 has set B(1) =
    /// Mach, B(2) = beta and A(129) = Reynolds number per unit length).
    /// The wing-body overlay M07O07 follows in a later checkpoint.
    pub fn subsonic(
        &mut self,
        point: &FlightPoint,
        experimental: Option<&Namelist>,
    ) -> Result<WingAero> {
        self.wing.fill(UNUSED);
        self.c.fill(UNUSED);
        self.d.fill(UNUSED);
        self.stb.fill(UNUSED);
        self.mach = point.mach;
        self.i = point.mach_index + 1;
        self.b[1] = point.mach;
        self.b[2] = (1.0 - point.mach * point.mach).abs().sqrt();
        self.a[129] = point.reynolds;
        self.exsubt(experimental);
        self.a[131] = self.ain[self.i + 20];
        self.a[132] = self.ain[self.i + 40];
        self.m15o17()?;
        self.cdrag()?;
        // M31O37 (CACALC is empty in the original).
        self.cmalph()?;
        let kwing = self.exsubt(experimental);
        self.slopes(kwing)?;
        self.exsubt(experimental);
        self.m29o35();
        self.sublat()?;
        self.result()
    }

    fn result(&mut self) -> Result<WingAero> {
        let n = self.alphas.len();
        let take = |base: usize| -> Result<Vec<f32>> {
            let v = self.wing[base..base + n].to_vec();
            finite(&v)?;
            Ok(v)
        };
        Ok(WingAero {
            cd: take(1)?,
            cl: take(21)?,
            cm: take(41)?,
            cn: take(61)?,
            ca: take(81)?,
            cla: take(101)?,
            cma: take(121)?,
            cyb: take(141)?,
            cnb: take(161)?,
            clb: take(181)?,
            messages: std::mem::take(&mut self.messages),
            work: WingWork {
                a: self.a[1..].to_vec(),
                b: self.b[1..].to_vec(),
                c: self.c[1..].to_vec(),
                d: self.d[1..].to_vec(),
                stb: self.stb[1..].to_vec(),
                wingin: self.ain[1..].to_vec(),
            },
        })
    }
}
