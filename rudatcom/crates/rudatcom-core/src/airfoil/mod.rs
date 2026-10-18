//! Airfoil section module: port of M50O62 (overlay 50) and its helpers.
//!
//! For each lifting surface with a planform, the section is taken from a NACA
//! card (DECODE/AIRFOL/COORDx), user coordinates (TYPEIN 1) or a user mean
//! line and thickness (TYPEIN 2). Weber's method (IDEAL/SLOPE) then gives the
//! ideal and zero-lift properties, lift-curve slopes and crest-critical Mach
//! number, MAXCL the section CLmax, and SECO fills every still-UNUSED input
//! slot of the xxSCHR namelist, exactly as the original does before WTGEOM.
//!
//! `Section` mirrors the scratch COMMON blocks (/IWING/ X, /IHT/, /IVT/,
//! /IBH/, /IBWH/, /IBWHV/, /IBV/). INIZ resets only part of them per surface;
//! the coordinate arrays deliberately persist across surfaces, as in Fortran.
//! Report printing is deferred to the output writer; messages become case
//! warnings and the original's STOP becomes an error.

mod aero;
mod coords;
mod decode;

use crate::{
    input::{Case, Diagnostic, UNUSED},
    interp::{tbfunx_unordered, Limits},
    math::{finite, powr, MathError},
    tables::RAD,
};

#[derive(Debug, thiserror::Error)]
pub enum AirfoilError {
    #[error("airfoil section: {0}")]
    Invalid(String),
    #[error("airfoil section calculation: {0}")]
    Numerical(#[from] MathError),
}
pub type Result<T> = std::result::Result<T, AirfoilError>;

/// Number of points in the section coordinate arrays (X(60) etc.).
const NP: usize = 60;
/// Weber collocation points (XN = 32 in IDEAL/SLOPE).
const NW: usize = 32;

/// Weber thickness/camber terms (/IBV/), indexed 1-based like the Fortran
/// (element 0 unused) so the IDEAL/SLOPE formulas read like the source.
#[derive(Debug, Clone)]
struct Weber {
    zt: [f32; NW + 1],
    zc: [f32; NW + 1],
    st1: [f32; NW + 1],
    st2: [f32; NW + 1],
    st3: [f32; NW + 1],
    st4: [f32; NW + 1],
    st5: [f32; NW + 1],
    /// Collocation abscissae X(I) = (cos(THNU)+1)/2.
    x: [f32; NW + 1],
    thnu: [f32; NW + 1],
}

impl Default for Weber {
    fn default() -> Self {
        let z = [0.0; NW + 1];
        Self {
            zt: z,
            zc: z,
            st1: z,
            st2: z,
            st3: z,
            st4: z,
            st5: z,
            x: z,
            thnu: z,
        }
    }
}

/// Section scratch state shared by the M50O62 routines.
#[derive(Debug, Clone)]
struct Section {
    /// Chordwise stations X (/IWING/).
    x: [f32; NP],
    /// Upper/lower abscissae XU, XL (/IHT/).
    xu: [f32; NP],
    xl: [f32; NP],
    /// User (or CORDSP) upper/lower ordinates YU, YL (/IHT/).
    yu: [f32; NP],
    yl: [f32; NP],
    /// Generated upper/lower ordinates YUN, YLN (/IVT/).
    yun: [f32; NP],
    yln: [f32; NP],
    /// Half thickness and mean line (/IBH/), reset by INIZ.
    thn: [f32; NP],
    cam: [f32; NP],
    a0: f32,
    /// Crest-critical location, Mach number and lift-curve slope.
    xc: f32,
    mcc: f32,
    clcc: f32,
    xac: [f32; 20],
    /// Point count shared through /IBW/ (DECODE's grid size or NPTS).
    n: usize,
    /// NACA digit fields I, J, K, II, JJ, KK, III, JJJ, KKK, LLL.
    digits: [i32; 10],
    /// /IBWH/: ideal and zero-lift angles, ideal CL, CM about c/4, slopes.
    ai: f32,
    alo: f32,
    cli: f32,
    cmco4: f32,
    cla0: f32,
    cla: [f32; 20],
    clmax0: f32,
    clmax: [f32; 20],
    /// /IBWHV/: leading-edge radius, max thickness and its/camber's location.
    rho: f32,
    tmax: f32,
    deltay: f32,
    xovc: f32,
    tovc: f32,
    covc: f32,
    cxvc: f32,
    camber: bool,
    /// Section mean aerodynamic chord used for Reynolds numbers (SECI).
    cbar: f32,
    /// Current surface's NACA card, 80 blank-padded columns (NACA(1..80)).
    card: [u8; 80],
    weber: Weber,
    warnings: Vec<String>,
}

impl Section {
    fn new() -> Self {
        Self {
            x: [0.0; NP],
            xu: [0.0; NP],
            xl: [0.0; NP],
            yu: [0.0; NP],
            yl: [0.0; NP],
            yun: [0.0; NP],
            yln: [0.0; NP],
            thn: [UNUSED; NP],
            cam: [UNUSED; NP],
            a0: UNUSED,
            xc: UNUSED,
            mcc: UNUSED,
            clcc: UNUSED,
            xac: [UNUSED; 20],
            n: 0,
            digits: [0; 10],
            ai: UNUSED,
            alo: UNUSED,
            cli: UNUSED,
            cmco4: UNUSED,
            cla0: UNUSED,
            cla: [UNUSED; 20],
            clmax0: UNUSED,
            clmax: [UNUSED; 20],
            rho: UNUSED,
            tmax: UNUSED,
            deltay: UNUSED,
            xovc: UNUSED,
            tovc: UNUSED,
            covc: UNUSED,
            cxvc: UNUSED,
            camber: false,
            cbar: UNUSED,
            card: [b' '; 80],
            weber: Weber::default(),
            warnings: Vec::new(),
        }
    }

    /// INIZ (plus M50O62's CAMBER reset): clear /IBH/ THN..XAC, /IBWH/ and
    /// /IBWHV/ for the next surface, keeping the coordinate arrays.
    fn iniz(&mut self) {
        let keep = std::mem::replace(self, Self::new());
        self.x = keep.x;
        self.xu = keep.xu;
        self.xl = keep.xl;
        self.yu = keep.yu;
        self.yl = keep.yl;
        self.yun = keep.yun;
        self.yln = keep.yln;
        self.n = keep.n;
        self.digits = keep.digits;
        self.cbar = keep.cbar;
        self.weber = keep.weber;
        self.warnings = keep.warnings;
    }

    /// Card column `col` (1-based), as NACA(col).
    fn col(&self, col: usize) -> u8 {
        self.card[col - 1]
    }
}

/// The four lifting surfaces in M50O62 order with their NACA card letters.
const SURFACES: [(&str, char, &str); 4] = [
    ("WG", 'W', "WING"),
    ("HT", 'H', "H.T."),
    ("VT", 'V', "V.T."),
    ("VF", 'F', "VFIN"),
];

/// A surface's xxIN(1..) view of its xxSCHR namelist storage.
///
/// VFIN(1) follows VTIN(154) and TVTIN(8) in the shared block, so VFIN(k)
/// is namelist location 162 + k.
struct SlotView {
    index: usize,
    offset: usize,
}

impl SlotView {
    fn get(&self, case: &Case, slot: usize) -> f32 {
        case.namelists[self.index].values[slot - 1 + self.offset]
    }
    fn set(&self, case: &mut Case, slot: usize, value: f32) {
        case.namelists[self.index].values[slot - 1 + self.offset] = value;
    }
    /// Store only into slots the user left UNUSED (the `IF(A(n).EQ.UNUSED)` idiom).
    fn fill(&self, case: &mut Case, slot: usize, value: f32) {
        if self.get(case, slot) == UNUSED {
            self.set(case, slot, value);
        }
    }
}

fn namelist_index(case: &Case, name: &str) -> usize {
    case.namelists
        .iter()
        .position(|n| n.def.name == name)
        .expect("airfoil namelist schema")
}

/// Raw (possibly UNUSED) value of a named variable, element `i`.
fn raw(case: &Case, list: &str, var: &str, i: usize) -> f32 {
    let n = &case.namelists[namelist_index(case, list)];
    n.get(var).expect("airfoil variable schema")[i]
}

/// Port of M50O62: compute section characteristics for every surface and
/// fill unspecified xxSCHR inputs. Operates on the CONV-converted case.
pub fn apply_sections(case: &mut Case) -> Result<()> {
    let fltcon = case.namelists[namelist_index(case, "FLTCON")]
        .values
        .clone();
    let mut s = Section::new();
    for (surface, &(prefix, letter, label)) in SURFACES.iter().enumerate() {
        s.iniz();
        let plnf = namelist_index(case, &format!("{prefix}PLNF"));
        // xxIN(15) holds the planform TYPE string only when xxPLNF was read.
        if !case.namelists[plnf].present {
            continue;
        }
        let view = SlotView {
            index: namelist_index(case, &format!("{prefix}SCHR")),
            offset: if prefix == "VF" { 162 } else { 0 },
        };
        s.card = [b' '; 80];
        // Each NACA card overwrites its surface's NACA(80) array: last one wins.
        if let Some(card) = case.naca.iter().rev().find(|c| c.surface == letter) {
            for (dst, b) in s.card.iter_mut().zip(card.text.bytes()) {
                *dst = b;
            }
        }
        let (atype, l) = seci(&mut s, case, prefix, plnf, &view)?;
        // The VT has no leading-edge-radius input; the VF reads the VT's value.
        s.rho = match prefix {
            "WG" | "HT" => view.get(case, 62),
            "VF" => case.namelists[namelist_index(case, "VTSCHR")].values[61],
            _ => UNUSED,
        };
        if atype < 0.0 {
            s.warnings.push(format!(
                "{label} NOT STRAIGHT TAPERED. UNIFORM SECTION ASSUMED."
            ));
        }
        let nacain = s.card.starts_with(b"NACA");
        if nacain && atype.abs() <= UNUSED {
            s.airfol(case, &view)?;
        } else if atype.abs() <= UNUSED {
            continue;
        } else {
            // TYPEIN 1 (coordinates) -> IVAL 0; TYPEIN 2 (mean line) -> IVAL 1.
            let ival = (atype.abs() - 0.5) as i32;
            // XYCORD(IVAL, IN) also doubles THN, prints it and halves it again:
            // an exact round trip, so only the geometry is computed here.
            s.xycord(ival)?;
        }
        // Label 1070: thickness/camber maxima (the latest increase wins).
        for m in 1..l {
            if s.thn[m] > s.thn[m - 1] {
                s.tovc = s.thn[m];
                s.xovc = s.x[m];
            }
            if s.cam[m] > s.cam[m - 1] {
                s.covc = s.cam[m];
                s.cxvc = s.x[m];
            }
        }
        s.camber = s.covc > UNUSED;
        if s.tmax == UNUSED {
            s.tmax = 2.0 * s.tovc;
        }
        if s.rho == UNUSED {
            s.rho = (s.x[1] * s.x[1] + s.thn[1] * s.thn[1]) / (2.0 * s.x[1]);
        }
        s.dely()?;
        s.theory(&fltcon)?;
        s.maxcl(&fltcon)?;
        seco(&s, case, &view, atype, &fltcon);
        if surface == 0 {
            cslope(&s, case, &view)?;
        }
    }
    for message in std::mem::take(&mut s.warnings) {
        case.warnings.push(Diagnostic {
            line: 0,
            message,
            card: String::new(),
        });
    }
    for n in &case.namelists {
        finite(&n.values)?;
    }
    Ok(())
}

/// SECI: load the user section definition and compute the section CBAR.
/// Returns ATYPE (negated for non-straight-tapered planforms) and the point
/// count L used by M50O62's own loop (60 for NACA sections).
fn seci(
    s: &mut Section,
    case: &Case,
    prefix: &str,
    plnf: usize,
    view: &SlotView,
) -> Result<(f32, usize)> {
    let schr = format!("{prefix}SCHR");
    let mut atype = raw(case, &schr, "TYPEIN", 0);
    let mut l = NP;
    if atype > UNUSED {
        l = (raw(case, &schr, "NPTS", 0) + 0.5) as i32 as usize;
        if !(1..=50).contains(&l) {
            return Err(AirfoilError::Invalid(format!(
                "{schr}: NPTS must be between 1 and 50"
            )));
        }
        for m in 0..l {
            s.x[m] = raw(case, &schr, "XCORD", m);
        }
        if atype == 1.0 {
            for m in 0..l {
                s.yu[m] = raw(case, &schr, "YUPPER", m);
                s.yl[m] = raw(case, &schr, "YLOWER", m);
            }
        } else if atype == 2.0 {
            for m in 0..l {
                s.cam[m] = raw(case, &schr, "MEAN", m);
                s.thn[m] = raw(case, &schr, "THICK", m) / 2.0;
            }
        } else {
            // The original would continue with stale coordinates.
            return Err(AirfoilError::Invalid(format!(
                "{schr}: TYPEIN must be 1 (coordinates) or 2 (mean line and thickness)"
            )));
        }
    }
    s.n = l;
    let pl = &case.namelists[plnf];
    // xxIN(15) is the TYPE string; only 'STRA' (TYPE 1) is straight tapered.
    if pl.scalar("TYPE") != Some(1.0) {
        atype = -atype;
    }
    for m in 0..20 {
        s.cla[m] = view.get(case, 21 + m);
    }
    let get = |name: &str| pl.get(name).expect("planform schema")[0];
    let chrdtp = get("CHRDTP");
    let mut sspnop = get("SSPNOP");
    let sspne = get("SSPNE");
    let sspn = get("SSPN");
    let mut chrdbp = get("CHRDBP");
    let chrdr = get("CHRDR");
    if sspnop <= 10.0 * UNUSED {
        chrdbp = chrdtp;
        sspnop = 0.0;
    }
    // Inboard/outboard panel MACs weighted by panel area (left-to-right f32).
    let mut tapri = chrdbp / chrdr;
    let crei = chrdr * (tapri + (1.0 - tapri) * (sspne - sspnop) / (sspn - sspnop));
    tapri = chrdbp / crei;
    let tapro = if chrdbp != 0.0 { chrdtp / chrdbp } else { 0.0 };
    let creo = chrdbp;
    let cbari = 2.0 * crei * (1.0 + tapri + tapri * tapri) / (3.0 * (1.0 + tapri));
    let cbaro = 2.0 * creo * (1.0 + tapro + tapro * tapro) / (3.0 * (1.0 + tapro));
    let areai = (sspn - sspnop) * crei * (1.0 + tapri);
    let areao = sspnop * creo * (1.0 + tapro);
    s.cbar = (cbari * areai + cbaro * areao) / (areai + areao);
    Ok((atype, l))
}

/// SECO: copy computed characteristics into the surface's UNUSED input slots.
fn seco(s: &Section, case: &mut Case, view: &SlotView, atype: f32, fltcon: &[f32]) {
    view.fill(case, 16, 2.0 * s.tovc);
    view.fill(case, 17, s.deltay);
    view.fill(case, 18, s.xovc);
    view.fill(case, 19, s.cli);
    view.fill(case, 20, s.ai);
    let nmach = ((fltcon[0] + 0.5) as i32).clamp(0, 20) as usize;
    for m in 0..nmach {
        view.fill(case, m + 21, s.cla[m]);
        view.fill(case, m + 41, s.clmax[m]);
        view.fill(case, m + 72, s.xac[m]);
    }
    view.fill(case, 61, s.cmco4);
    view.fill(case, 62, s.rho);
    // The LOGICAL CAMBER is always stored, as 1.0 or 0.0.
    view.set(case, 64, if s.camber { 1.0 } else { 0.0 });
    view.fill(case, 68, s.clmax0);
    view.fill(case, 69, s.cla0);
    view.fill(case, 10, s.alo);
    view.fill(case, 93, s.covc);
    if atype < 0.0 {
        // Non-straight-tapered planforms also get the outboard-panel values.
        view.fill(case, 63, s.rho);
        view.fill(case, 65, 2.0 * s.tovc);
        view.fill(case, 66, s.xovc);
        view.fill(case, 67, s.cmco4);
    }
}

/// CSLOPE: wing section surface slopes (deg) at 0-100% chord, taken on the
/// side facing the horizontal tail; only when an HTPLNF was read. Surface x
/// may reverse near a cambered leading edge, which the original tolerates.
fn cslope(s: &Section, case: &mut Case, view: &SlotView) -> Result<()> {
    if !case.namelists[namelist_index(case, "HTPLNF")].present {
        return Ok(());
    }
    let syn = &case.namelists[namelist_index(case, "SYNTHS")].values;
    let (zw, zh) = (syn[2], syn[6]);
    let n = s.n;
    for (i, at) in [0.0, 0.2, 0.4, 0.6, 0.8, 1.0].into_iter().enumerate() {
        let slot = 95 + i;
        if view.get(case, slot) != UNUSED {
            continue;
        }
        let slope = if zh > zw {
            tbfunx_unordered(&s.xu[..n], &s.yun[..n], at, Limits::default())?.derivative
        } else {
            -tbfunx_unordered(&s.xl[..n], &s.yln[..n], at, Limits::default())?.derivative
        };
        if slope != UNUSED {
            view.set(case, slot, slope.atan() * RAD);
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests;
