//! Post-processing of the namelists: port of the checks in INPUTB.
//!
//! The original rewrites some inputs after reading them (clamping, deriving body
//! geometry, resolving mutually exclusive namelists); doing it here means later
//! stages see the same values the Fortran code would.

use super::{Case, Diagnostic, Namelist, UNUSED};

/// `PI` as stored in the original's CONSNT block (single precision).
// Same literal as the original's DATA statement, kept verbatim for traceability.
#[allow(clippy::approx_constant, clippy::excessive_precision)]
const PI: f32 = 3.141_592_654;

/// Which speed regimes the requested Mach numbers fall in.
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub struct Regimes {
    pub subsonic: bool,
    pub transonic: bool,
    pub supersonic: bool,
}

fn index(namelists: &[Namelist], name: &str) -> usize {
    namelists
        .iter()
        .position(|n| n.def.name == name)
        .expect("namelist table")
}

fn set(n: &mut Namelist, var: &str, value: f32) {
    if let Some(v) = n.def.vars.iter().find(|v| v.name == var) {
        if v.loc > 0 {
            n.values[v.loc - 1] = value;
        }
    }
}

/// Checks applied immediately after a namelist is read (per occurrence).
pub(super) fn on_namelist_read(namelists: &mut [Namelist], idx: usize) {
    match namelists[idx].def.name {
        "FLTCON" => {
            let n = &mut namelists[idx];
            // TR is a flag: any supplied value means 1.
            let tr = if n.get("TR").is_some_and(|v| v[0] > UNUSED) {
                1.0
            } else {
                0.0
            };
            set(n, "TR", tr);
            let stmach = n.get("STMACH").map_or(UNUSED, |v| v[0]);
            set(n, "STMACH", stmach.clamp(0.6, 0.99));
            // TSMACH defaults to 1.4 and is limited to [1.01, 1.4].
            let ts = n.get("TSMACH").map_or(UNUSED, |v| v[0]);
            let ts = if ts <= UNUSED {
                1.4
            } else {
                ts.clamp(1.01, 1.4)
            };
            set(n, "TSMACH", ts);
        }
        // The later of SYMFLP/ASYFLP wins; the loser's shared flap storage is reset.
        "SYMFLP" => reset_other(namelists, "ASYFLP"),
        "ASYFLP" => reset_other(namelists, "SYMFLP"),
        "PROPWR" => namelists[index(namelists, "JETPWR")].present = false,
        "JETPWR" => namelists[index(namelists, "PROPWR")].present = false,
        "LARWB" => {
            // XNAM13 copies these into OPTION immediately, so a later OPTINS
            // can override them and CONV applies the model scale only once.
            let area = namelists[idx].get("SREF").expect("LARWB SREF")[0];
            let chord = namelists[idx].get("L").expect("LARWB L")[0];
            let options = index(namelists, "OPTINS");
            set(&mut namelists[options], "SREF", area);
            set(&mut namelists[options], "CBARR", chord);
        }
        "VFSCHR" => {
            // VFSCHR's ALPHAO is declared at VTIN(10) (not VFIN(10)), so it
            // lands in the vertical tail's section storage.
            let alphao = namelists[idx].values[9];
            if alphao != UNUSED {
                let vt = index(namelists, "VTSCHR");
                namelists[vt].values[9] = alphao;
                namelists[idx].values[9] = UNUSED;
            }
        }
        _ => {}
    }
}

fn reset_other(namelists: &mut [Namelist], other: &str) {
    let o = index(namelists, other);
    if namelists[o].present {
        namelists[o].present = false;
        // F(1..69) of the shared FLAPIN block.
        namelists[o].values[..69].fill(UNUSED);
    }
}

/// Planform TYPE (1=straight, 2=double-delta, 3=cranked, 4=curved) as used downstream.
fn planform_type(raw: Option<f32>) -> (u8, bool) {
    match raw {
        None => (1, false),
        Some(t) if t >= 5.0 => (1, true),
        Some(t) => (((t + 0.5) as i32).clamp(1, 4) as u8, false),
    }
}

/// Tail/fin TYPE as CHECK resolves it: XTYPE(INT(TYPE+.5)) with no default
/// and no range check, so a missing or out-of-range TYPE reads outside the
/// four names. Such a surface matches none of them (in particular it is not
/// straight tapered); rudatcom represents that as 0.
fn tail_type(raw: Option<f32>) -> u8 {
    match raw.map(|t| (t + 0.5) as i32) {
        Some(t @ 1..=4) => t as u8,
        _ => 0,
    }
}

/// Whole-case checks after all namelists are read.
pub fn finalize_case(case: &mut Case) {
    let mut warnings = Vec::new();

    for name in ["WGPLNF", "HTPLNF", "VTPLNF", "VFPLNF"] {
        let i = index(&case.namelists, name);
        if !case.namelists[i].present {
            continue;
        }
        let raw = case.namelists[i].scalar("TYPE");
        if name != "WGPLNF" {
            // Only the wing gets CHECK's default and range message.
            set(&mut case.namelists[i], "TYPE", f32::from(tail_type(raw)));
            continue;
        }
        let (t, bad) = planform_type(raw);
        if bad {
            warnings.push(Diagnostic {
                line: 0,
                message: format!("{name}: ERROR TYPE CANNOT BE GREATER THAN 4. TYPE SET TO 1."),
                card: String::new(),
            });
        }
        set(&mut case.namelists[i], "TYPE", f32::from(t));
    }

    let b = index(&case.namelists, "BODY");
    if case.namelists[b].present {
        if let Some(d) = derive_body(&mut case.namelists[b]) {
            case.diagnostics.push(d);
        }
    }

    let f = index(&case.namelists, "FLTCON");
    if case.namelists[f].present {
        let fl = &case.namelists[f];
        let nmach = fl.scalar("NMACH").map_or(0, |n| (n + 0.5) as usize);
        let (st, ts) = (
            fl.scalar("STMACH").unwrap_or(0.6),
            fl.scalar("TSMACH").unwrap_or(1.4),
        );
        let mut r = Regimes::default();
        for &m in fl.get("MACH").unwrap_or(&[]).iter().take(nmach) {
            r.subsonic |= m <= st;
            r.supersonic |= m >= ts;
            r.transonic |= m > st && m < ts;
        }
        case.regimes = r;
    }
    case.warnings = warnings;
}

/// BODY: only (X,R), (X,S) or (X,R,S,P) are accepted; derive the missing radii/areas/perimeters.
fn derive_body(n: &mut Namelist) -> Option<Diagnostic> {
    let nx = n.scalar("NX").map_or(0, |v| (v + 0.5) as usize).min(20);
    let mut ellip = n.get("ELLIP").map_or(UNUSED, |v| v[0]);
    if ellip == 0.0 || ellip == UNUSED {
        ellip = 1.0;
    }
    set(n, "ELLIP", ellip);
    let first = |n: &Namelist, v: &str| n.get(v).map_or(UNUSED, |s| s[0]);
    let (r1, s1, p1) = (first(n, "R"), first(n, "S"), first(n, "P"));
    let mut r = n.get("R")?.to_vec();
    let mut s = n.get("S")?.to_vec();
    let mut p = n.get("P")?.to_vec();

    if r1 != UNUSED && s1 == UNUSED && p1 == UNUSED {
        // Half width given.
        for k in 0..nx {
            s[k] = PI * (r[k] * r[k]) * ellip;
            p[k] = 2.0 * PI * r[k] * ((ellip * ellip + 1.0) / 2.0).sqrt();
            r[k] *= ellip.sqrt();
        }
    } else if s1 != UNUSED && r1 == UNUSED && p1 == UNUSED {
        // Cross-sectional area given.
        #[allow(clippy::needless_range_loop)] // indexes r, s and p in lockstep
        for k in 0..nx {
            r[k] = (s[k] / PI).sqrt();
            p[k] = 2.0 * PI * ((s[k] / ellip) * (ellip * ellip + 1.0) / 2.0 / PI).sqrt();
        }
    } else if r1 != UNUSED && s1 != UNUSED && p1 != UNUSED {
        #[allow(clippy::needless_range_loop)]
        for k in 0..nx {
            r[k] *= ellip.sqrt();
        }
    } else {
        return Some(Diagnostic {
            line: 0,
            message:
                "THE BODY IS NOT ADEQUATELY DEFINED: specify X and R, X and S, or X, R, S and P"
                    .into(),
            card: String::new(),
        });
    }
    for (name, vals) in [("R", r), ("S", s), ("P", p)] {
        if let Some(v) = n.def.vars.iter().find(|v| v.name == name) {
            n.values[v.loc - 1..v.loc - 1 + vals.len()].copy_from_slice(&vals);
        }
    }
    None
}
