//! Wing-alone aerodynamics. Ports the subsonic path of MAIN01:
//! M15O17 (CALCA0, WTLIFT, LIFTCF, CLMXBS), M03O03 (CDRAG, FIG53A), M31O37
//! (CMALPH, FWDXAC, CACALC and the slope pass), M29O35's wing part and
//! M17O21's SUBLAT, plus the main program's once-per-case CLMCH0 pre-pass.
//!
//! State model: the original keeps the wing work arrays A(195)/B(49)
//! (COMMON /WINGD/) and WINGIN alive for the whole case, and several
//! routines write to them (CMALPH, CDRAG, M29O35's defaults), so
//! [`WingCase`] holds 1-based copies that persist across flight points.
//! WING, C, D, STB, SLG and TRA are reset to UNUSED per point, as INITZ1
//! does. Index 0 of every array is unused so subscripts read like the
//! Fortran. The supersonic (MAIN04) and transonic (MAIN03) paths live in
//! [`supersonic`] and [`transonic`].
//!
//! Reuses [`WingCase`] for the subsonic horizontal tail: the
//! original calls the same routines with AHT/BHT/HTIN/HT (CLMCH0 ITTY=1,
//! M16O20, M05O05, M33O41, M29O35's HT part, SUBLAT's second call), so a
//! [`Surface`] tag selects the few tail-specific differences. The vertical
//! tail, ventral fin and twin vertical panels are in [`fin`].

mod data;
mod drag;
mod fin;
mod lateral;
mod lift;
mod moment;
mod supersonic;
mod transonic;

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

pub use fin::{FinAero, FinWork, Fins};

/// Which lifting surface a [`WingCase`] models: the wing (A, B, WINGIN,
/// WING, STB) or the horizontal tail (AHT, BHT, HTIN, HT, STBH).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Surface {
    Wing,
    HorizontalTail,
}

impl Surface {
    /// (section namelist, SYNTHS incidence index: ALIW or ALIH).
    fn names(self) -> (&'static str, usize) {
        match self {
            Surface::Wing => ("WGSCHR", 3),
            Surface::HorizontalTail => ("HTSCHR", 7),
        }
    }

    /// EXPR value slots (0-based XNAM23 AH indices): CD, CL, CM, CLA and CMA
    /// tables, ALPO/ALPL and ACLM/CLM (stored in TRA(37)/(38) or TRAH).
    fn expr_slots(self) -> ([usize; 5], [usize; 4]) {
        match self {
            Surface::Wing => ([100, 120, 140, 160, 180], [461, 462, 465, 466]),
            Surface::HorizontalTail => ([200, 220, 240, 260, 280], [463, 464, 467, 468]),
        }
    }
}

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
    /// A(1..195), B(1..49), C(1..51), D(1..55), STB(1..135), WINGIN(1..91)
    /// (AHT, BHT, CHT, DHT, STBH and HTIN(1..154) for the horizontal tail).
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
    /// SLG(1..141); all UNUSED at subsonic and transonic points.
    pub slg: Vec<f32>,
    /// TRA(1..108), the transonic work array (UNUSED in other regimes).
    pub tra: Vec<f32>,
    /// SECOND(1..23) (COMMON /LEVEL2/), the per-case DOL2 pre-pass results;
    /// SECOND(15) DONE and SECOND(16) DOL2 hold LOGICAL bit patterns.
    pub second: Vec<f32>,
}

/// Per-case wing state (COMMON /WINGD/, /WINGI/, /IWING/, /WHAERO/ C and
/// D, /SBETA/ STB) plus the case constants the routines read; for the
/// horizontal tail the same fields hold AHT, BHT, HTIN, HT, CHT, DHT, STBH
/// and TRAH.
pub struct WingCase {
    surface: Surface,
    a: [f32; 196],
    b: [f32; 50],
    /// WINGIN(1..91) or HTIN(1..154).
    ain: [f32; 155],
    wing: [f32; 401],
    c: [f32; 52],
    d: [f32; 56],
    stb: [f32; 136],
    /// COMMON /SUPWH/ SLG(141): supersonic work slots (see `supersonic`).
    slg: [f32; 142],
    /// COMMON /SBETA/ TRA(108): transonic work slots (see `transonic`).
    tra: [f32; 109],
    /// COMMON /LEVEL2/ SECOND(23), kept for the whole case.
    second: [f32; 24],
    /// EXPR ALPOW/ALPLW (/EXPER/): EXPDAT resets them per point, EXSUBT
    /// sets them from the point's EXPR namelists.
    alpow: f32,
    alplw: f32,
    /// A as WTGEOM leaves it, which M18O22 restores per supersonic point.
    a_geometry: [f32; 196],
    /// SYNTHS XCG, XW and ALIW (SYNA(1), SYNA(2), SYNA(4)).
    syna: (f32, f32, f32),
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

/// WINGIN/HTIN/VTIN/VFIN(1..15) after WTGEOM (converted units, CHRDBP =
/// tip chord for a single panel). The planform XNAMs never store SWAFP, so
/// slot 10 is left for the section namelist's ALPHAO.
fn planform_inputs(p: &surface::Planform) -> [f32; 15] {
    [
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
        f32::from(p.kind),
    ]
}

impl WingCase {
    /// Build the case state (WINGIN, A after M02O02, B after SETUP1) and run
    /// the main program's CLMCH0 pre-pass when the case has subsonic or
    /// transonic Mach numbers. `None` without a wing.
    pub fn new(geometry: &Geometry, points: &[FlightPoint]) -> Result<Option<Self>> {
        Self::build(geometry, points, Surface::Wing)
    }

    /// The horizontal tail's case state (AHT, BHT after SETUP1 with ALIH,
    /// HTIN) after the main program's CLMCH0 ITTY=1 pre-pass (M16O20).
    /// `None` without HTPLNF.
    pub fn horizontal_tail(geometry: &Geometry, points: &[FlightPoint]) -> Result<Option<Self>> {
        Self::build(geometry, points, Surface::HorizontalTail)
    }

    fn build(
        geometry: &Geometry,
        points: &[FlightPoint],
        kind_of: Surface,
    ) -> Result<Option<Self>> {
        let planform_surface = match kind_of {
            Surface::Wing => &geometry.wing,
            Surface::HorizontalTail => &geometry.horizontal_tail,
        };
        let Some(surface) = planform_surface else {
            return Ok(None);
        };
        let (section, incidence) = kind_of.names();
        let case = &geometry.inputs.case;
        let sc = case.namelist(section).expect("section schema");
        let fl = case.namelist("FLTCON").expect("FLTCON schema");
        let syn = case.namelist("SYNTHS").expect("SYNTHS schema");
        let p = &surface.planform;
        let kind = p.kind;
        let mut ain = [UNUSED; 155];
        ain[1..16].copy_from_slice(&planform_inputs(p));
        // WINGIN(10) and WINGIN(16..91) are the WGSCHR block in namelist
        // location order (XNAM8 copies HTIN(16..154) from HTSCHR alike).
        ain[10] = sc.values[9];
        let last = match kind_of {
            Surface::Wing => 92,
            Surface::HorizontalTail => 155,
        };
        ain[16..last].copy_from_slice(&sc.values[15..last - 1]);
        // CAMBER is LOGICAL in WINGIN(64): unset reads as .FALSE. (0).
        let camber = ain[64] == 1.0;
        ain[64] = if camber { 1.0 } else { 0.0 };
        let mut a = [0.0; 196];
        a[1..].copy_from_slice(&surface.coefficients[..195]);
        let nalpha = ((fl.values[1] + 0.5) as i32).clamp(0, 20) as usize;
        let alphas = fl.values[22..22 + nalpha].to_vec();
        // SETUP1: B(J+22) = alpha + ALIW (BHT with ALIH; an UNUSED
        // incidence adds 1e-30).
        let mut b = [UNUSED; 50];
        for (j, &alpha) in alphas.iter().enumerate() {
            b[j + 23] = alpha + syn.values[incidence];
        }
        let reference = &geometry.reference;
        let mut state = WingCase {
            surface: kind_of,
            a,
            b,
            ain,
            wing: [UNUSED; 401],
            c: [UNUSED; 52],
            d: [UNUSED; 56],
            stb: [UNUSED; 136],
            slg: [UNUSED; 142],
            tra: [UNUSED; 109],
            // INITZE: SECOND = UNUSED, then DONE = DOL2 = .FALSE. (0 bits).
            second: {
                let mut s = [UNUSED; 24];
                s[15] = 0.0;
                s[16] = 0.0;
                s
            },
            alpow: UNUSED,
            alplw: UNUSED,
            a_geometry: a,
            // Read only by the supersonic/transonic wing paths.
            syna: (syn.values[0], syn.values[1], syn.values[3]),
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

    /// EXSUBT for this surface: given EXPR values replace the CD, CL, CM,
    /// CLA and CMA entries (wing slots 100..180, HT slots 200..280),
    /// ALPO/ALPL and TRA(37)/TRA(38) (ACLM/CLM). Returns KWING (KHT), true
    /// when any of the five first entries is given.
    fn exsubt(&mut self, experimental: Option<&Namelist>) -> bool {
        let Some(data) = experimental else {
            return false;
        };
        let v = &data.values;
        let (tables, [alpo, alpl, aclm, clm]) = self.surface.expr_slots();
        let given = |slot: usize, current: f32| if v[slot] != UNUSED { v[slot] } else { current };
        self.alpow = given(alpo, self.alpow);
        self.alplw = given(alpl, self.alplw);
        self.tra[37] = given(aclm, self.tra[37]);
        self.tra[38] = given(clm, self.tra[38]);
        let mut kwing = false;
        for (slot, base) in tables.into_iter().zip([1, 21, 41, 101, 121]) {
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
    /// The wing-body overlay M07O07 is still not implemented.
    pub fn subsonic(
        &mut self,
        point: &FlightPoint,
        experimental: Option<&Namelist>,
    ) -> Result<WingAero> {
        self.main00(point);
        self.main01(experimental)?;
        self.result()
    }

    /// MAIN00's per-point setup: Mach index, B(1) = Mach, B(2) = beta and
    /// A(129) = Reynolds number per unit length. EXPDAT (overlay 48) also
    /// resets ALPOW/ALPLW for every point before EXSUBT reads them.
    fn main00(&mut self, point: &FlightPoint) {
        self.mach = point.mach;
        self.i = point.mach_index + 1;
        self.b[1] = point.mach;
        self.b[2] = (1.0 - point.mach * point.mach).abs().sqrt();
        self.a[129] = point.reynolds;
        self.alpow = UNUSED;
        self.alplw = UNUSED;
    }

    /// M51O63 (INITZ1): the per-point arrays start as UNUSED.
    fn initz1(&mut self) {
        self.wing.fill(UNUSED);
        self.c.fill(UNUSED);
        self.d.fill(UNUSED);
        self.stb.fill(UNUSED);
        self.slg.fill(UNUSED);
        self.tra.fill(UNUSED);
    }

    /// MAIN01's wing overlays at `self.mach` with B(1)/B(2) as set by the
    /// caller (MAIN00, or SETUP2 in the transonic pre-pass).
    fn main01(&mut self, experimental: Option<&Namelist>) -> Result<()> {
        self.initz1();
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
        self.sublat()
    }

    fn result(&mut self) -> Result<WingAero> {
        let n = self.alphas.len();
        let inputs = match self.surface {
            Surface::Wing => 92,
            Surface::HorizontalTail => 155,
        };
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
                wingin: self.ain[1..inputs].to_vec(),
                slg: self.slg[1..].to_vec(),
                tra: self.tra[1..].to_vec(),
                second: self.second[1..].to_vec(),
            },
        })
    }
}
