//! SETUP1/WTGEOM/ARCLSS lifting-surface geometry and ANGLES trigonometry.
//!
//! The working arrays below deliberately use one-based indices so expressions
//! remain traceable to WTGEOM. Only the public coefficient array is zero-based.

use super::{GeometryError, Result};
use crate::{input::UNUSED, interp, math::finite};

/// Straight, double-delta, cranked or curved planform, in normalized feet.
#[derive(Debug, Clone)]
pub struct Planform {
    pub tip_chord: f32,
    pub outboard_semispan: f32,
    pub exposed_semispan: f32,
    pub semispan: f32,
    pub break_chord: f32,
    pub root_chord: f32,
    pub inboard_sweep: f32,
    pub outboard_sweep: f32,
    pub chord_station: f32,
    pub twist: f32,
    pub dihedral_break: f32,
    pub inboard_dihedral: f32,
    pub outboard_dihedral: f32,
    pub kind: u8,
    pub thickness_station: f32,
    pub outboard_thickness_station: f32,
    pub aspect_ratio_class: f32,
}

/// Geometry shared by wings, horizontal tails and single vertical/ventral fins.
#[derive(Debug, Clone)]
pub struct SurfaceGeometry {
    /// A(1)..A(195). Uncomputed aerodynamic slots retain UNUSED.
    pub coefficients: [f32; 195],
    /// WTGEOM replaces the breakpoint chord by the tip chord for a single panel.
    pub planform: Planform,
}

impl SurfaceGeometry {
    /// Original one-based A offset; useful when porting downstream methods.
    pub fn coefficient(&self, offset: usize) -> Option<f32> {
        offset
            .checked_sub(1)
            .and_then(|i| self.coefficients.get(i))
            .copied()
    }
    pub fn exposed_area(&self) -> f32 {
        self.coefficients[2]
    }
    pub fn area(&self) -> f32 {
        self.coefficients[3]
    }
    pub fn mean_aerodynamic_chord(&self) -> f32 {
        self.coefficients[121]
    }
    pub fn exposed_mean_aerodynamic_chord(&self) -> f32 {
        self.coefficients[15]
    }
    pub fn aspect_ratio(&self) -> f32 {
        self.coefficients[119]
    }
    pub fn leading_edge_tangent(&self) -> f32 {
        self.coefficients[61]
    }

    /// M02O02/M18O22: WTGEOM computes a mirrored surface, then halves the
    /// specified areas and aspect ratios for a single fin (not its MAC).
    pub(super) fn single_fin(&mut self, z: f32) -> Result<()> {
        for i in [1, 2, 3, 4, 5, 6, 7, 119, 120, 163, 167, 168] {
            if self.coefficients[i - 1] != UNUSED {
                self.coefficients[i - 1] /= 2.0;
            }
        }
        for i in [130, 133, 136] {
            self.coefficients[i - 1] += z;
        }
        finite(&self.coefficients)?;
        Ok(())
    }
}

// ANGLES has its own rounded constants, distinct from CONSNT's DEG/RAD.
#[allow(clippy::excessive_precision, clippy::approx_constant)]
pub(crate) fn angles(mode: u8, arg: &mut [f32]) -> Result<()> {
    const CON: f32 = 1.74532925199433E-2;
    const HPI: f32 = 1.57079632679489;
    const OPI: f32 = 3.14159265358979;
    const TPI: f32 = 6.28318530717959;
    const EPS: f32 = 1.52587890625000E-5;
    const SPE: f32 = 1.04857600000000E6;
    let zero = |a: &mut [f32]| {
        a.fill(0.0);
        a[3] = 1.0;
    };
    let (mut a2, trig) = if mode == 1 {
        (arg[0] * CON, true)
    } else {
        let (a3, a4) = if mode == 4 {
            let mut c = arg[3];
            if c.abs() >= 1.0 - EPS {
                c = c.signum();
            }
            let s = (1.0 - c * c).sqrt();
            if c.abs() > EPS && (s - arg[2]).abs() <= EPS && c * arg[3] >= 0.0 {
                return Ok(());
            }
            (s, c)
        } else {
            let mut t = arg[4];
            if t.abs() > SPE {
                t = SPE.copysign(t);
            }
            let s = (t / (1.0 + t * t).sqrt()).abs().copysign(t);
            if s.abs() <= EPS {
                zero(arg);
                return Ok(());
            }
            if s.abs() >= 1.0 - EPS {
                (s, 0.0)
            } else {
                let c = (1.0 - s * s).sqrt();
                if (c - arg[3]).abs() <= EPS && s * arg[2] >= 0.0 {
                    return Ok(());
                }
                (s, c)
            }
        };
        if a4.abs() <= EPS {
            arg[2] = 1.0f32.copysign(a3);
            arg[3] = 0.0;
            arg[4] = SPE.copysign(a3);
            (HPI.copysign(a3), false)
        } else {
            let s = if a3.abs() <= EPS { 0.0 } else { a3 };
            arg[2] = s;
            arg[3] = a4;
            arg[4] = s / a4;
            (s.atan2(a4), false)
        }
    };
    a2 = (a2 + TPI) % TPI;
    if a2 > OPI {
        a2 -= TPI;
    }
    if (a2 - arg[5]).abs() <= EPS {
        return Ok(());
    }
    if a2.abs() <= EPS {
        zero(arg);
        return Ok(());
    }
    arg[0] = a2 / CON;
    arg[1] = a2;
    if trig {
        arg[2] = a2.sin();
        if arg[2].abs() <= EPS {
            arg[2] = 0.0;
        }
        arg[3] = a2.cos();
        if arg[3].abs() <= EPS {
            arg[3] = 0.0;
            arg[4] = SPE.copysign(a2);
        } else {
            arg[4] = arg[2] / arg[3];
        }
    }
    arg[5] = a2;
    finite(arg)?;
    Ok(())
}

fn zero_angle(a: &mut [f32]) {
    a.fill(0.0);
    a[3] = 1.0;
}

/// Port of WTGEOM, with explicit validation of divisions that were undefined
/// for missing/degenerate geometry in the original.
pub fn wtgeom(planform: &Planform) -> Result<SurfaceGeometry> {
    let mut p = planform.clone();
    finite(&[
        p.tip_chord,
        p.outboard_semispan,
        p.exposed_semispan,
        p.semispan,
        p.break_chord,
        p.root_chord,
        p.inboard_sweep,
        p.outboard_sweep,
        p.chord_station,
        p.twist,
        p.dihedral_break,
        p.inboard_dihedral,
        p.outboard_dihedral,
        p.thickness_station,
        p.outboard_thickness_station,
        p.aspect_ratio_class,
    ])?;
    let no_outer = p.outboard_semispan < 10.0 * UNUSED;
    // TYPE 0 is a tail/fin whose TYPE resolved outside the four names.
    if p.kind > 4
        || p.root_chord <= UNUSED
        || p.tip_chord < 0.0
        || p.outboard_semispan < 0.0
        || p.exposed_semispan <= p.outboard_semispan
        || p.semispan < p.exposed_semispan
        || (!no_outer && p.break_chord <= UNUSED)
    {
        return Err(GeometryError::Invalid(
            "WTGEOM requires TYPE 0..4, positive chords and 0 <= SSPNOP < SSPNE <= SSPN".into(),
        ));
    }
    if no_outer {
        p.break_chord = p.tip_chord;
    }
    let mut a = [UNUSED; 196];
    // INITZE, XNAM4/7/9/20 and SETUP1 establish the angle caches.
    for start in [106, 112, 138, 147, 153] {
        zero_angle(&mut a[start..start + 6]);
    }
    a[106] = p.inboard_sweep;
    a[112] = p.outboard_sweep;
    angles(1, &mut a[106..112])?;
    angles(1, &mut a[112..118])?;
    angles(1, &mut a[138..144])?;
    a[174] = p.thickness_station;
    a[21] = p.semispan - p.outboard_semispan;
    a[23] = p.exposed_semispan - p.outboard_semispan;
    a[19] = a[23] / a[21];
    a[25] = p.break_chord / p.root_chord;
    a[10] = p.root_chord * (a[25] + (1.0 - a[25]) * a[19]);
    if a[10] <= 0.0 {
        return Err(GeometryError::Invalid(
            "WTGEOM exposed root chord is zero".into(),
        ));
    }
    a[26] = p.break_chord / a[10];
    a[28] = if p.break_chord != 0.0 {
        p.tip_chord / p.break_chord
    } else {
        1.0
    };
    a[27] = a[26] * a[28];
    a[118] = p.tip_chord / p.root_chord;
    a[1] = (a[10] + p.break_chord) * a[23];
    a[2] = (p.break_chord + p.tip_chord) * p.outboard_semispan;
    a[3] = a[1] + a[2];
    a[119] = (p.root_chord + p.break_chord) * a[21];
    a[4] = a[119] + a[2];
    let ar = |span: f32, area: f32| 4.0 * (span * span) / area;
    a[5] = ar(a[23], a[1]);
    a[6] = if no_outer {
        1.0
    } else {
        ar(p.outboard_semispan, a[2])
    };
    a[7] = ar(p.exposed_semispan, a[3]);
    a[120] = ar(p.semispan, a[4]);
    let slope = |taper: f32, aspect: f32| 4.0 * (1.0 - taper) / (aspect * (1.0 + taper));
    let ci = slope(a[26], a[5]);
    let co = if no_outer { 0.0 } else { slope(a[28], a[6]) };
    for i in 0..5 {
        let (e, inner, outer, tmpi, tmpo) = if i == 4 {
            (
                175,
                187,
                181,
                p.chord_station - a[174],
                p.chord_station - p.outboard_thickness_station,
            )
        } else {
            let station = [0.0, 0.25, 0.5, 1.0][i];
            (
                34 + i * 6,
                58 + i * 6,
                82 + i * 6,
                p.chord_station - station,
                p.chord_station - station,
            )
        };
        if tmpi.abs() < 1e-6 {
            for j in 0..6 {
                a[inner + j] = a[106 + j];
                a[outer + j] = a[112 + j];
            }
        } else {
            a[inner + 4] = ci * tmpi + a[110];
            angles(5, &mut a[inner..inner + 6])?;
            if no_outer {
                zero_angle(&mut a[outer..outer + 6]);
            } else {
                a[outer + 4] = co * tmpo + a[116];
                angles(5, &mut a[outer..outer + 6])?;
            }
        }
        if no_outer {
            for j in 0..6 {
                a[e + j] = a[inner + j];
            }
        } else {
            a[e + 3] = (a[1] * a[inner + 3] + a[2] * a[outer + 3]) / a[3];
            angles(4, &mut a[e..e + 6])?;
        }
    }
    let mac = |c: f32, taper: f32| 2.0 * c * (1.0 + taper + taper * taper) / (3.0 * (1.0 + taper));
    a[15] = mac(a[10], a[26]);
    a[121] = mac(p.root_chord, a[25]);
    a[17] = mac(p.break_chord, a[28]);
    a[16] = (a[1] * a[15] + a[2] * a[17]) / a[3];
    a[122] = (a[119] * a[121] + a[2] * a[17]) / a[4];
    a[32] = a[23] * (1.0 + 2.0 * a[26]) / (3.0 * (1.0 + a[26]));
    a[33] = p.outboard_semispan * (1.0 + 2.0 * a[28]) / (3.0 * (1.0 + a[28])) + a[23];
    a[31] = (a[1] * a[32] + a[2] * a[33]) / a[3];
    a[30] = a[16] / 2.0
        + (a[1] * a[32] * a[62] + a[2] * (a[23] * a[62] + (a[33] - a[23]) * a[86])) / a[3];
    a[18] = (a[23] * a[62] + p.outboard_semispan * a[86]) / a[10];
    let a1 = a[23] * a[62];
    let a2 = p.outboard_semispan * a[86] + a1;
    let a3 = a[23] * a[80];
    let a4 = p.outboard_semispan * a[104] + a3;
    a[29] = a[10] + 0.0f32.max(a4).max(a3) - 0.0f32.min(a2).min(a1);
    a[162] = a[23] / p.exposed_semispan;
    a[163] = ar(a[21], a[119]);
    a[164] = a[23] / 2.0;
    a[165] = a[164] + p.outboard_semispan;
    if !no_outer {
        a[166] = p.tip_chord + a[165] * ((p.break_chord - p.tip_chord) / p.outboard_semispan);
        a[167] = (a[166] + p.tip_chord) * a[165];
        a[168] = ar(a[165], a[167]);
        a[169] = p.tip_chord / a[166];
    }
    a[130] = a[21] * (1.0 + 2.0 * a[25]) / (3.0 * (1.0 + a[25]));
    a[133] = if no_outer {
        0.0
    } else {
        p.outboard_semispan * (1.0 + 2.0 * a[28]) / (3.0 * (1.0 + a[28])) + a[21]
    };
    a[136] = (a[119] * a[130] + a[2] * a[133]) / a[4];
    a[195] = (a[119] * a[130] * a[62] + a[2] * (a[21] * a[62] + (a[133] - a[21]) * a[86])) / a[4];
    a[161] = a[122] / 4.0 + a[195];
    // ARCLSS, figure 4.1.3.4-24B.
    a[128] = if p.aspect_ratio_class == UNUSED {
        3.5
    } else {
        p.aspect_ratio_class.clamp(3.0, 4.0)
    };
    a[123] = interp::tbfunx(
        &[0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0],
        &[
            0.0, 0.225, 0.47, 0.496, 0.43, 0.32, 0.21, 0.125, 0.075, 0.0475, 0.0,
        ],
        a[27],
        interp::Limits::default(),
    )?
    .value;
    angles(1, &mut a[34..40])?;
    a[124] = (a[123] + 1.0) * a[37];
    a[125] = a[128] / a[124];
    finite(&a[1..])?;
    let mut coefficients = [UNUSED; 195];
    coefficients.copy_from_slice(&a[1..]);
    Ok(SurfaceGeometry {
        coefficients,
        planform: p,
    })
}
