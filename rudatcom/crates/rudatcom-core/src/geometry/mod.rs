//! Geometry stage: CONV, SETUP1/WTGEOM/ARCLSS, SYNDIM and INFTGM.
//!
//! `Geometry::from_case` leaves parsed/SAVE inputs unchanged and exposes both
//! named dimensions and traceable COMMON-array coefficients for later methods.

pub mod body;
pub mod convert;
pub mod surface;

use crate::{
    airfoil::{apply_sections, AirfoilError},
    input::{Case, Namelist, UNUSED},
    math::{finite, MathError},
    tables::RAD,
};
pub use body::{BodyGeometry, BodyStations, EquallySpacedBody};
pub use convert::{convert_case, ConvertedCase};
pub use surface::{wtgeom, Planform, SurfaceGeometry};

#[derive(Debug, thiserror::Error)]
pub enum GeometryError {
    #[error("invalid geometry: {0}")]
    Invalid(String),
    #[error("geometry calculation: {0}")]
    Numerical(#[from] MathError),
    #[error(transparent)]
    Airfoil(#[from] AirfoilError),
}
pub type Result<T> = std::result::Result<T, GeometryError>;

#[derive(Debug, Clone)]
pub struct ReferenceQuantities {
    /// None means neither supplied nor computable without a wing.
    pub area: Option<f32>,
    pub chord: Option<f32>,
    pub span: Option<f32>,
    pub roughness: f32,
}

#[derive(Debug, Clone)]
pub struct SynthesisDimensions {
    /// BD(66), BD(67), BD(68), BD(78:80), BD(87).
    pub wing_body_le_offset: f32,
    pub cg_from_wing_body_le: f32,
    pub wing_height_intercept: f32,
    pub incidence_sine: f32,
    pub incidence_cosine: f32,
    pub incidence_tangent: f32,
    pub body_width_at_wing: f32,
}

#[derive(Debug, Clone)]
pub struct Geometry {
    pub inputs: ConvertedCase,
    pub reference: ReferenceQuantities,
    pub wing: Option<SurfaceGeometry>,
    pub horizontal_tail: Option<SurfaceGeometry>,
    pub vertical_tail: Option<SurfaceGeometry>,
    pub ventral_fin: Option<SurfaceGeometry>,
    pub body: Option<BodyGeometry>,
    pub synthesis: Option<SynthesisDimensions>,
}

fn list<'a>(case: &'a Case, name: &str) -> &'a Namelist {
    case.namelist(name).expect("geometry namelist schema")
}
fn supplied(n: &Namelist, name: &str) -> f32 {
    n.scalar(name).unwrap_or(UNUSED)
}

fn surface(case: &Case, prefix: &str) -> Result<Option<SurfaceGeometry>> {
    let pl = list(case, &format!("{prefix}PLNF"));
    if !pl.present {
        return Ok(None);
    }
    let section = list(case, &format!("{prefix}SCHR"));
    let slot = ["WG", "HT", "VT", "VF"]
        .iter()
        .position(|&p| p == prefix)
        .expect("surface prefix");
    let get = |name| supplied(pl, name);
    let p = Planform {
        tip_chord: get("CHRDTP"),
        outboard_semispan: get("SSPNOP"),
        exposed_semispan: get("SSPNE"),
        semispan: get("SSPN"),
        break_chord: get("CHRDBP"),
        root_chord: get("CHRDR"),
        inboard_sweep: case.geometry_sweeps[slot][0],
        outboard_sweep: case.geometry_sweeps[slot][1],
        chord_station: get("CHSTAT"),
        twist: get("TWISTA"),
        dihedral_break: get("SSPNDD"),
        inboard_dihedral: get("DHDADI"),
        outboard_dihedral: get("DHDADO"),
        kind: pl.scalar("TYPE").unwrap_or(1.0) as u8,
        thickness_station: supplied(section, "XOVC"),
        outboard_thickness_station: supplied(section, "XOVCO"),
        aspect_ratio_class: supplied(section, "ARCL"),
    };
    wtgeom(&p)
        .map(Some)
        .map_err(|e| GeometryError::Invalid(format!("{}: {e}", pl.def.name)))
}

impl Geometry {
    pub fn from_case(case: &Case) -> Result<Self> {
        let mut inputs = convert_case(case)?;
        // M50O62 runs between CONV and M02O02, filling UNUSED xxSCHR section
        // inputs (e.g. thickness used by WTGEOM) from NACA cards/coordinates.
        apply_sections(&mut inputs.case)?;
        let normalized = &inputs.case;
        // M02O02 bypasses the ordinary lifting-surface path for LARWB.
        let lifting_surface = |prefix| {
            if list(normalized, "LARWB").present {
                Ok(None)
            } else {
                surface(normalized, prefix)
            }
        };
        let mut wing = lifting_surface("WG")?;
        let mut horizontal_tail = lifting_surface("HT")?;
        let mut vertical_tail = lifting_surface("VT")?;
        let mut ventral_fin = lifting_surface("VF")?;
        let syn = list(normalized, "SYNTHS");
        if let Some(v) = &mut vertical_tail {
            v.single_fin(supplied(syn, "ZV"))?;
        }
        if let Some(v) = &mut ventral_fin {
            v.single_fin(supplied(syn, "ZVF"))?;
        }
        let body = if list(normalized, "BODY").present {
            Some(body::from_namelist(list(normalized, "BODY"))?)
        } else {
            None
        };
        let options = list(normalized, "OPTINS");
        let area = options
            .scalar("SREF")
            .or_else(|| wing.as_ref().map(SurfaceGeometry::area));
        let chord = options
            .scalar("CBARR")
            .or_else(|| wing.as_ref().map(SurfaceGeometry::mean_aerodynamic_chord));
        let span = options
            .scalar("BLREF")
            .or_else(|| wing.as_ref().map(|w| 2.0 * w.planform.semispan));
        let roughness = options.scalar("ROUGFC").unwrap_or(1.6e-4);
        for (name, value) in [
            ("SREF", area),
            ("CBARR", chord),
            ("BLREF", span),
            ("ROUGFC", Some(roughness)),
        ] {
            if value.is_some_and(|v| v <= 0.0 || !v.is_finite()) {
                return Err(GeometryError::Invalid(format!(
                    "{name} must be positive and finite"
                )));
            }
        }
        let synthesis = if let Some(w) = &mut wing {
            let result = syndim(w, horizontal_tail.as_mut(), syn)?;
            if body.is_some() {
                inftgm(w, horizontal_tail.as_ref(), &mut inputs.case)?;
            }
            Some(result)
        } else {
            if body.is_some() && horizontal_tail.is_some() {
                let syn = inputs
                    .case
                    .namelists
                    .iter_mut()
                    .find(|n| n.def.name == "SYNTHS")
                    .expect("SYNTHS schema");
                rotate_tail(syn)?;
            }
            None
        };
        Ok(Self {
            inputs,
            reference: ReferenceQuantities {
                area,
                chord,
                span,
                roughness,
            },
            wing,
            horizontal_tail,
            vertical_tail,
            ventral_fin,
            body,
            synthesis,
        })
    }
}

/// SYNDIM: retain BD offsets independently of the mutable synthesis coordinates.
fn syndim(
    wing: &mut SurfaceGeometry,
    horizontal: Option<&mut SurfaceGeometry>,
    syn: &Namelist,
) -> Result<SynthesisDimensions> {
    let p = &wing.planform;
    let width = 2.0 * (p.semispan - p.exposed_semispan);
    let incidence = supplied(syn, "ALIW");
    let sine = (incidence / RAD).sin();
    let cosine = (incidence / RAD).cos();
    let tangent = sine / cosine;
    let offset = 0.5 * width * wing.leading_edge_tangent() * cosine;
    let cg = supplied(syn, "XCG") - (supplied(syn, "XW") + offset);
    let height = supplied(syn, "ZW") - supplied(syn, "XW") * tangent;
    wing.coefficients[172] = cg;
    if let Some(h) = horizontal {
        let le = supplied(syn, "XH")
            + (h.planform.semispan - h.planform.exposed_semispan)
                * h.leading_edge_tangent()
                * (supplied(syn, "ALIH") / RAD).cos();
        h.coefficients[172] = supplied(syn, "XCG") - le;
    }
    finite(&[width, sine, cosine, tangent, offset, cg, height])?;
    Ok(SynthesisDimensions {
        wing_body_le_offset: offset,
        cg_from_wing_body_le: cg,
        wing_height_intercept: height,
        incidence_sine: sine,
        incidence_cosine: cosine,
        incidence_tangent: tangent,
        body_width_at_wing: width,
    })
}

/// INFTGM: downwash synthesis geometry; hinge rotation affects XH/ZH, not the
/// pre-rotation CG arm already stored by SYNDIM.
fn rotate_tail(syn: &mut Namelist) -> Result<()> {
    if let Some(hinge) = syn.scalar("HINAX") {
        let angle = supplied(syn, "ALIH") / RAD;
        let xh = supplied(syn, "XH");
        let zh = supplied(syn, "ZH") + angle.sin() * (hinge - xh);
        let xh = hinge * (1.0 - angle.cos()) + xh * angle.cos();
        finite(&[xh, zh])?;
        syn.values[5] = xh;
        syn.values[6] = zh;
    }
    Ok(())
}

fn inftgm(
    wing: &mut SurfaceGeometry,
    horizontal: Option<&SurfaceGeometry>,
    case: &mut Case,
) -> Result<()> {
    let syn = case
        .namelists
        .iter_mut()
        .find(|n| n.def.name == "SYNTHS")
        .expect("SYNTHS schema");
    if horizontal.is_some() {
        rotate_tail(syn)?;
    }
    let xw = supplied(syn, "XW");
    let zw = supplied(syn, "ZW");
    let iw = supplied(syn, "ALIW");
    let ih = supplied(syn, "ALIH");
    let xh = supplied(syn, "XH");
    let zh = supplied(syn, "ZH");
    let a = &mut wing.coefficients;
    if let Some(h) = horizontal {
        let cw = (iw / RAD).cos();
        let ch = (ih / RAD).cos();
        let sh = (ih / RAD).sin();
        let sw = (iw / RAD).sin();
        let tw = sw / cw;
        let xbrsth = h.coefficients[29] - h.coefficients[15] / 4.0;
        let dxstar = (h.planform.semispan - h.planform.exposed_semispan) * h.leading_edge_tangent();
        let dxbh = xbrsth + dxstar;
        a[192] = xh - xw - wing.planform.root_chord * cw;
        a[193] = a[192] + dxbh * ch;
        let zph = zh - dxbh * sh - zw + wing.planform.root_chord * sw;
        let dlh = zph * tw;
        a[23] = (a[193] - dlh) * cw;
        a[21] = wing.planform.root_chord - a[194];
        a[8] = a[28] + (wing.planform.semispan - wing.planform.exposed_semispan) * a[37];
        a[11] = zph / cw + (a[193] - dlh) * sw;
        a[10] = (a[11] / a[23]).atan();
    }
    if a[51] >= 0.0 {
        a[7] = 1.0;
    } else if a[111] == UNUSED {
        a[7] = a[109] / a[12] + wing.planform.chord_station;
    } else {
        let mut cp = 0.0;
        let mut ip = 33;
        let mut ic = 33;
        let mut exact = false;
        for i in 0..3 {
            cp = i as f32;
            if a[ic].abs() <= 1e-3 {
                a[7] = 0.25 * cp;
                exact = true;
                break;
            }
            ip = ic;
            ic += 6;
            if a[ip] > 0.0 && a[ic] < 0.0 {
                break;
            }
        }
        if !exact {
            a[7] = 0.25 * (cp + (cp + 1.0) * a[ip] / (a[ip] - a[ic]));
        }
    }
    finite(a)?;
    Ok(())
}

#[cfg(test)]
mod tests;
