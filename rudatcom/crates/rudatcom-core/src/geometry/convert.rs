//! CONV: convert and scale an owned copy of the input, then shift the body nose.
//! Keeping this separate from parsing prevents double conversion of SAVE cases.

use super::{GeometryError, Result};
use crate::input::{Case, Diagnostic, DimUnit, Namelist, UNUSED};
use crate::math::finite;

#[derive(Debug, Clone)]
pub struct ConvertedCase {
    pub case: Case,
    /// Body nose location in feet before translating the coordinate origin.
    pub nose_origin: f32,
    pub scale: f32,
}

fn named(n: &mut Namelist, name: &str, mut f: impl FnMut(f32) -> f32) {
    let var = n
        .def
        .vars
        .iter()
        .find(|v| v.name == name)
        .expect("geometry namelist schema");
    assert!(var.loc > 0);
    for value in &mut n.values[var.loc - 1..var.loc - 1 + var.dim] {
        if *value != UNUSED {
            *value = f(*value);
        }
    }
}

fn slots(
    n: &mut Namelist,
    indices: impl IntoIterator<Item = usize>,
    mut f: impl FnMut(f32) -> f32,
) {
    // Individual XNAM views can be shorter than their shared COMMON block
    // (SYMFLP exposes 117 of FLAPIN's 138 slots).
    let size = n.values.len();
    for i in indices.into_iter().filter(|&i| i <= size) {
        let value = &mut n.values[i - 1];
        if *value != UNUSED {
            *value = f(*value);
        }
    }
}

fn set(n: &mut Namelist, name: &str, value: f32) {
    let var = n
        .def
        .vars
        .iter()
        .find(|v| v.name == name)
        .expect("geometry namelist schema");
    n.values[var.loc - 1] = value;
}

/// CHECK clips flap spans in input units, before SCALE and DIM conversion.
fn check_geometry(case: &mut Case) {
    let surface = case
        .namelist("HTPLNF")
        .filter(|n| n.present)
        .unwrap_or_else(|| case.namelist("WGPLNF").expect("WGPLNF schema"));
    let inboard =
        surface.scalar("SSPN").unwrap_or(UNUSED) - surface.scalar("SSPNE").unwrap_or(UNUSED);
    for n in &mut case.namelists {
        match n.def.name {
            "BODY" => {
                let method = if n.scalar("METHOD").unwrap_or(UNUSED) <= 1.5 {
                    1.0
                } else {
                    2.0
                };
                set(n, "METHOD", method);
            }
            "WGSCHR" | "HTSCHR" | "VTSCHR" | "VFSCHR" => {
                let class = n.scalar("ARCL").unwrap_or(3.5).clamp(3.0, 4.0);
                set(n, "ARCL", class);
            }
            "SYMFLP" | "ASYFLP" if n.present => {
                if n.def.name == "ASYFLP"
                    && (n.scalar("STYPE").unwrap_or(UNUSED) - 5.0).abs() <= 0.1
                {
                    continue;
                }
                let span = n.scalar("SPANFI").unwrap_or(UNUSED);
                if span < inboard {
                    set(n, "SPANFI", inboard);
                    case.warnings.push(Diagnostic { line: 0, card: String::new(),
                                message: format!("{}: flap inboard span {span} is inside the body; redefined to {inboard} (input units)", n.def.name) });
                }
            }
            _ => {}
        }
    }
}

/// CONV plus CHECK's SCALE default; angles and dimensionless inputs are untouched.
pub fn convert_case(input: &Case) -> Result<ConvertedCase> {
    if !input.is_runnable() {
        return Err(GeometryError::Invalid(
            "case has input diagnostics; fix them before computing geometry".into(),
        ));
    }
    let mut case = input.clone();
    for n in &case.namelists {
        finite(&n.values)?;
    }
    check_geometry(&mut case);
    let raw_scale = case
        .namelist("SYNTHS")
        .and_then(|n| n.scalar("SCALE"))
        .unwrap_or(UNUSED);
    let scale = if raw_scale <= UNUSED { 1.0 } else { raw_scale };
    let (xl, xa, xr, xp, xt, xw, xf) = match input.dim {
        DimUnit::Feet => (1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0),
        DimUnit::Inches => (12.0, 144.0, 1.0, 144.0, 1.0, 1.0, 1.0),
        DimUnit::Meters => (0.3048, 0.09290304, 0.3048, 0.0208854, 1.8, 0.2248089, 2.54),
        DimUnit::Centimeters => (30.48, 929.0304, 0.3048, 208.854, 1.8, 0.2248089, 2.54),
    };
    let ascale = scale * scale;
    for n in &mut case.namelists {
        if input.dim != DimUnit::Feet || scale != 1.0 {
            match n.def.name {
                "FLTCON" => {
                    named(n, "RNNUB", |v| v * xr);
                    for name in ["ALT", "VINF"] {
                        named(n, name, |v| v / xl);
                    }
                    named(n, "PINF", |v| v * xp);
                    named(n, "TINF", |v| v * xt);
                    named(n, "WT", |v| v * xw);
                }
                "GRNDEF" => named(n, "GRDHT", |v| v / xl),
                "OPTINS" => {
                    named(n, "SREF", |v| v * ascale / xa);
                    named(n, "CBARR", |v| v * scale / xl);
                    named(n, "BLREF", |v| v * scale / xl);
                    named(n, "ROUGFC", |v| v / xf);
                }
                "SYNTHS" => {
                    slots(
                        n,
                        (1..=19).filter(|i| ![4, 8, 10, 13, 18, 19].contains(i)),
                        |v| v * scale / xl,
                    );
                }
                "BODY" => {
                    for name in ["X", "P", "R", "ZU", "ZL", "BLN", "BLA", "DS"] {
                        named(n, name, |v| v * scale / xl);
                    }
                    named(n, "S", |v| v * ascale / xa);
                }
                "WGPLNF" | "HTPLNF" | "VTPLNF" | "VFPLNF" => {
                    for name in [
                        "CHRDTP", "SSPNOP", "SSPNE", "SSPN", "CHRDBP", "CHRDR", "SSPNDD",
                    ] {
                        named(n, name, |v| v * scale / xl);
                    }
                    // XNAM7/9/20 remap the per-Mach synthesis inputs to COMMON.
                    let names: &[&str] = match n.def.name {
                        "HTPLNF" => &["RLPH", "SHB", "SEXT"],
                        "VTPLNF" | "VFPLNF" => &["SVWB", "SVB", "SVHB"],
                        _ => &[],
                    };
                    for &name in names {
                        if name == "RLPH" {
                            named(n, name, |v| v * scale / xl);
                        } else {
                            named(n, name, |v| v * ascale / xa);
                        }
                    }
                }
                "PROPWR" | "JETPWR" => {
                    slots(n, [4, 5, 6, 8, 9, 10, 16, 17, 18, 24, 27, 28], |v| {
                        v * scale / xl
                    });
                    slots(n, [19], |v| v * ascale / xa);
                    slots(n, [21], |v| v / xl);
                    slots(n, [22, 23], |v| v * xt);
                    slots(n, [25, 26], |v| v * xp);
                }
                "TVTPAN" => {
                    slots(n, [155, 156, 157, 158, 161, 162], |v| v * scale / xl);
                    slots(n, [159], |v| v * ascale / xa);
                }
                "SYMFLP" if n.present => {
                    slots(
                        n,
                        (12..=125).filter(|i| {
                            !(16..=38).contains(i)
                                && !(61..=84).contains(i)
                                && !(105..=114).contains(i)
                                && *i != 117
                        }),
                        |v| v * scale / xl,
                    );
                    slots(n, [133, 134], |v| v * xl);
                    slots(n, [135], |v| v * xp);
                }
                "ASYFLP" if n.present => slots(n, 12..=15, |v| v * scale / xl),
                "TRNJET" if n.present => {
                    slots(n, 12..=21, |v| v * xw);
                    slots(n, [34, 38], |v| v / xl);
                }
                "HYPEFF" if n.present => slots(n, [1, 2, 4], |v| v / xl),
                "LARWB" => {
                    slots(n, [2, 4, 9, 11, 18, 19].map(|i| i + 29), |v| v / xa);
                    slots(n, [1, 8, 10, 12, 13, 15, 20, 21].map(|i| i + 29), |v| {
                        v / xl
                    });
                }
                _ => {}
            }
        }
        if n.def.name == "SYNTHS" {
            // Preserve the applied SCALE for report headings, as CONV does.
            n.values[12] = scale;
        }
    }
    let body = case.namelist("BODY").expect("BODY schema");
    let nose_origin = body.get("X").expect("BODY X")[0];
    if nose_origin.abs() > UNUSED {
        let nx = super::body::station_count(body)?;
        for n in &mut case.namelists {
            match n.def.name {
                "BODY" => {
                    for x in &mut n.values[1..1 + nx] {
                        *x -= nose_origin;
                    }
                }
                "SYNTHS" => {
                    // The four mandatory coordinates are shifted even if unset.
                    for i in [1, 2, 6, 9] {
                        n.values[i - 1] -= nose_origin;
                    }
                    slots(n, [11, 12], |v| v - nose_origin);
                }
                "PROPWR" | "JETPWR" => {
                    for i in [4, 16, 18] {
                        n.values[i - 1] -= nose_origin;
                    }
                }
                _ => {}
            }
        }
    }
    // A converted copy is explicitly feet; SCALE is report metadata, not a
    // request to convert it again. Consumers should use Geometry::from_case.
    case.dim = DimUnit::Feet;
    for n in &case.namelists {
        finite(&n.values)?;
    }
    Ok(ConvertedCase {
        case,
        nose_origin,
        scale,
    })
}
