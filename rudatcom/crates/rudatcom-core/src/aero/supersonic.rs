//! Supersonic and hypersonic body-alone methods.
//!
//! * SYPBOD (called by M19O23): nose-cylinder lift/moment slopes from DATCOM
//!   figures 4.2.1.1-21/22 and 4.2.2.1-23..25, crossflow lift, and the
//!   friction + wave + interference + base zero-lift drag. Transonic points
//!   (MAIN03) run it in "transonic mode" at Mach 1.4, keeping only the
//!   slopes and drag.
//! * HYPBOD (called by M26O32 when FLTCON HYPERS and BODY METHOD = 1):
//!   Newtonian-impact slopes and a modified-Newtonian integration along the
//!   body (NASA TN D-176).
//!
//! Results keep the original COMMON /SUPBOD/ SBD(n) slot numbers in their
//! documentation. Slots the original leaves unset hold `UNUSED` (INITZ1
//! resets SBD for every flight point), so later arithmetic on them matches.
// Figure data are copied digit-for-digit from the original DATA statements.
#![allow(clippy::excessive_precision, clippy::approx_constant)]

use super::{body::Inputs, AeroError, Result};
use crate::{
    flight::{FlightPoint, Regime},
    input::UNUSED,
    interp::{self, Extrapolation, Limits},
    math::{finite, frustum_volume, powr, trapz},
    tables::{PI, RAD},
};

/// INTERX extrapolation flag: 0 holds, 1 is linear, 2 is quadratic.
fn flag(code: u8) -> Extrapolation {
    match code {
        0 => Extrapolation::Hold,
        1 => Extrapolation::Linear,
        _ => Extrapolation::Quadratic,
    }
}

/// INTERX with the original's (lower, upper) flag pair per axis; axes are
/// X1-fastest like the Fortran DATA tables.
fn interx(axes: &[&[f32]], values: &[f32], at: &[f32], flags: &[(u8, u8)]) -> Result<f32> {
    let limits: Vec<Limits> = flags
        .iter()
        .map(|&(lower, upper)| Limits {
            lower: flag(lower),
            upper: flag(upper),
        })
        .collect();
    Ok(interp::interx(axes, values, at, &limits)?.value)
}

fn last(x: &[f32], y: &[f32]) -> Result<f32> {
    Ok(*trapz(x, y)?.last().expect("nonempty"))
}

// FIG 4.1.5.1-27 roughness cutoff exponent vs Mach.
const X27M: [f32; 4] = [0.0, 1.0, 2.0, 3.0];
const X27I: [f32; 4] = [1.57780, 1.67221, 1.98509, 2.28874];
// FIG 4.2.2.1-23A ogive-cylinder XCP/LBP: X1 = beta/FN (or its inverse on
// the right side), X2 = FA/FN.
const T4218A_X1: [f32; 6] = [0., 0.2, 0.4, 0.6, 0.8, 1.0];
const T4218A_X2: [f32; 9] = [0.0, 0.4, 0.8, 1.2, 1.6, 2.0, 3.0, 4.0, 5.0];
#[rustfmt::skip]
const DL218A: [f32; 54] = [
    0.543, 0.542, 0.541, 0.540, 0.534, 0.526,
    0.400, 0.409, 0.418, 0.430, 0.441, 0.450,
    0.305, 0.328, 0.350, 0.369, 0.387, 0.400,
    0.238, 0.265, 0.295, 0.318, 0.339, 0.356,
    0.198, 0.221, 0.246, 0.274, 0.298, 0.320,
    0.160, 0.185, 0.210, 0.239, 0.262, 0.288,
    0.065, 0.095, 0.122, 0.150, 0.177, 0.210,
    0.000, 0.005, 0.035, 0.062, 0.089, 0.130,
    0.000, 0.000, 0.000, 0.000, 0.002, 0.050,
];
#[rustfmt::skip]
const DR218A: [f32; 54] = [
    0.445, 0.464, 0.485, 0.500, 0.518, 0.526,
    0.448, 0.455, 0.460, 0.460, 0.459, 0.450,
    0.460, 0.449, 0.438, 0.424, 0.412, 0.400,
    0.450, 0.430, 0.412, 0.394, 0.375, 0.356,
    0.432, 0.410, 0.388, 0.365, 0.343, 0.320,
    0.420, 0.394, 0.369, 0.340, 0.314, 0.288,
    0.388, 0.354, 0.322, 0.278, 0.244, 0.210,
    0.357, 0.314, 0.273, 0.216, 0.171, 0.130,
    0.325, 0.274, 0.225, 0.154, 0.100, 0.050,
];
// FIG 4.2.2.1-23B cone-cylinder XCP/LBP (same arguments).
const T4218B_X1: [f32; 6] = [0., 0.2, 0.4, 0.6, 0.8, 1.0];
const T4218B_X2: [f32; 7] = [0.0, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0];
#[rustfmt::skip]
const DL218B: [f32; 42] = [
    0.665, 0.665, 0.665, 0.665, 0.665, 0.665,
    0.425, 0.492, 0.539, 0.550, 0.550, 0.550,
    0.330, 0.370, 0.405, 0.438, 0.459, 0.470,
    0.184, 0.215, 0.250, 0.284, 0.318, 0.350,
    0.060, 0.097, 0.133, 0.170, 0.206, 0.240,
    0.000, 0.000, 0.044, 0.083, 0.127, 0.170,
    0.000, 0.000, 0.000, 0.020, 0.063, 0.105,
];
#[rustfmt::skip]
const DR218B: [f32; 42] = [
    0.665, 0.665, 0.665, 0.665, 0.665, 0.665,
    0.480, 0.500, 0.519, 0.536, 0.546, 0.550,
    0.338, 0.388, 0.430, 0.458, 0.471, 0.470,
    0.338, 0.372, 0.394, 0.394, 0.375, 0.350,
    0.410, 0.375, 0.341, 0.308, 0.272, 0.240,
    0.377, 0.338, 0.294, 0.251, 0.211, 0.170,
    0.342, 0.300, 0.246, 0.194, 0.146, 0.100,
];
// FIG 4.2.2.1-24 boat-tail XCP: X1 = beta*tan(theta), X2 = D2/D1.
const T42119_X1: [f32; 11] = [0., 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5];
const T42119_X2: [f32; 7] = [0., 0.2, 0.4, 0.6, 0.8, 0.9, 1.];
#[rustfmt::skip]
const D42119: [f32; 77] = [
    0.320, 0.348, 0.365, 0.394, 0.439, 0.480, 0.504, 0.518, 0.529, 0.537, 0.541,
    0.376, 0.400, 0.429, 0.468, 0.518, 0.543, 0.558, 0.567, 0.574, 0.579, 0.582,
    0.417, 0.445, 0.486, 0.544, 0.573, 0.587, 0.596, 0.603, 0.607, 0.610, 0.611,
    0.440, 0.486, 0.564, 0.600, 0.614, 0.621, 0.626, 0.630, 0.632, 0.633, 0.634,
    0.476, 0.580, 0.627, 0.638, 0.643, 0.646, 0.649, 0.651, 0.652, 0.653, 0.653,
    0.485, 0.632, 0.648, 0.653, 0.655, 0.657, 0.659, 0.660, 0.661, 0.661, 0.661,
    0.667, 0.667, 0.667, 0.667, 0.667, 0.667, 0.667, 0.667, 0.667, 0.667, 0.667,
];
// FIG 4.2.2.1-25A flare moment CMAP: X1 = flare angle (deg), X2 = D1/D2.
const T4220A_X1: [f32; 18] = [
    5.0, 10., 15., 20., 25., 30., 35., 40., 45., 50., 55., 60., 65., 70., 75., 80., 85., 90.,
];
const T4220A_X2: [f32; 11] = [0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0];
#[rustfmt::skip]
const D4220A: [f32; 198] = [
    -4.03, -3.25, -2.48, -1.80, -1.40, -1.14, -0.95, -0.80, -0.67, -0.56,
    -0.47, -0.39, -0.32, -0.25, -0.20, -0.13, -0.07, 0.0,
    -4.03, -3.02, -2.10, -1.60, -1.24, -1.01, -0.85, -0.72, -0.61, -0.52,
    -0.44, -0.37, -0.30, -0.24, -0.19, -0.12, -0.06, 0.0,
    -4.03, -2.70, -1.80, -1.34, -1.08, -0.90, -0.77, -0.66, -0.57, -0.49,
    -0.42, -0.36, -0.29, -0.23, -0.18, -0.11, -0.06, 0.0,
    -4.03, -2.10, -1.46, -1.10, -0.90, -0.76, -0.67, -0.59, -0.51, -0.46,
    -0.40, -0.34, -0.28, -0.22, -0.17, -0.10, -0.06, 0.0,
    -4.03, -1.67, -1.15, -0.90, -0.73, -0.63, -0.57, -0.51, -0.46, -0.41,
    -0.36, -0.31, -0.26, -0.21, -0.16, -0.10, -0.05, 0.0,
    -2.40, -1.24, -0.85, -0.68, -0.58, -0.52, -0.48, -0.43, -0.40, -0.36,
    -0.32, -0.29, -0.24, -0.20, -0.15, -0.10, -0.05, 0.0,
    -1.60, -0.84, -0.61, -0.50, -0.44, -0.41, -0.38, -0.35, -0.33, -0.31,
    -0.28, -0.25, -0.22, -0.18, -0.14, -0.09, -0.04, 0.0,
    -0.90, -0.52, -0.39, -0.33, -0.31, -0.30, -0.29, -0.28, -0.27, -0.25,
    -0.23, -0.20, -0.18, -0.14, -0.11, -0.08, -0.04, 0.0,
    -0.40, -0.26, -0.22, -0.20, -0.20, -0.20, -0.19, -0.19, -0.18, -0.18,
    -0.17, -0.15, -0.13, -0.11, -0.08, -0.05, -0.03, 0.0,
    -0.12, -0.09, -0.08, -0.09, -0.09, -0.10, -0.10, -0.10, -0.10, -0.10,
    -0.09, -0.08, -0.07, -0.06, -0.04, -0.03, -0.02, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
];
// FIG 4.2.1.1-21A ogive-cylinder CNA: X1 = beta/FN, X2 = FA/FN.
const T4221A_X1: [f32; 11] = [0.0, 0.4, 0.6, 0.8, 1.0, 1.1, 1.25, 1.43, 1.667, 2.0, 10.0];
const T4221A_X2: [f32; 8] = [0.0, 0.4, 0.8, 1.2, 2.0, 3.0, 4.0, 5.0];
#[rustfmt::skip]
const D4221A: [f32; 88] = [
    2.25, 2.46, 2.42, 2.33, 2.21, 2.14, 2.07, 1.99, 1.91, 1.83, 1.43,
    2.25, 2.63, 2.69, 2.71, 2.66, 2.61, 2.55, 2.48, 2.39, 2.30, 1.85,
    2.25, 2.74, 2.87, 2.93, 2.93, 2.91, 2.87, 2.78, 2.68, 2.56, 1.96,
    2.25, 2.78, 2.95, 3.04, 3.09, 3.08, 3.06, 2.96, 2.86, 2.71, 1.96,
    2.25, 2.81, 3.01, 3.13, 3.25, 3.27, 3.25, 3.18, 3.06, 2.78, 1.38,
    2.25, 2.83, 3.04, 3.19, 3.31, 3.33, 3.33, 3.26, 3.14, 2.87, 1.52,
    2.25, 2.83, 3.04, 3.22, 3.34, 3.36, 3.35, 3.30, 3.18, 2.94, 1.74,
    2.25, 2.83, 3.04, 3.22, 3.34, 3.36, 3.35, 3.30, 3.18, 2.94, 1.74,
];
// FIG 4.2.1.1-21B cone-cylinder CNA (same arguments).
const T4221B_X1: [f32; 10] = [0.0, 0.4, 0.6, 0.678, 0.8, 0.9, 1.0, 1.25, 2.0, 10.0];
const T4221B_X2: [f32; 8] = [0.0, 0.4, 0.8, 1.2, 2.0, 3.0, 4.0, 5.0];
#[rustfmt::skip]
const D4221B: [f32; 80] = [
    2.08, 1.96, 1.87, 1.84, 1.84, 1.85, 1.85, 1.85, 1.85, 1.85,
    2.64, 2.59, 2.55, 2.53, 2.47, 2.42, 2.37, 2.35, 2.35, 2.43,
    2.62, 2.77, 2.79, 2.80, 2.80, 2.80, 2.78, 2.76, 2.76, 2.77,
    2.51, 2.84, 2.95, 2.98, 3.04, 3.07, 3.08, 3.04, 3.00, 2.94,
    2.35, 2.86, 3.04, 3.10, 3.20, 3.24, 3.24, 3.28, 3.32, 3.43,
    2.36, 2.90, 3.10, 3.18, 3.28, 3.30, 3.31, 3.38, 3.47, 3.62,
    2.37, 2.92, 3.14, 3.25, 3.35, 3.36, 3.37, 3.47, 3.61, 3.80,
    2.38, 2.95, 3.19, 3.33, 3.40, 3.41, 3.42, 3.56, 3.75, 3.98,
];
// FIG 4.2.1.1-22A boat-tail CNA increment vs FB/beta.
const T4222A: [f32; 8] = [0.0, 0.5, 1.0, 1.25, 1.50, 1.75, 2.0, 2.5];
const D4222A: [f32; 8] = [0.0, -0.88, -1.44, -1.64, -1.80, -1.90, -1.97, -2.00];
// FIG 4.2.1.1-22B flare CNA increment vs D2/D1.
const T4222B: [f32; 6] = [1.0, 1.4, 1.8, 2.2, 2.6, 3.0];
const D4222B: [f32; 6] = [0.0, 2.0, 4.5, 7.7, 11.6, 16.0];
// FIG 4.2.1.2-35B crossflow drag vs crossflow Mach (supersonic range).
const T4217B: [f32; 18] = [
    0.0, 0.25, 0.30, 0.35, 0.4, 0.5, 0.6, 0.7, 0.75, 0.80, 0.85, 1.0, 1.111, 1.25, 1.666, 2.5,
    3.33, 10.,
];
const D4217B: [f32; 18] = [
    1.2, 1.2, 1.215, 1.235, 1.265, 1.36, 1.5, 1.67, 1.735, 1.77, 1.79, 1.785, 1.74, 1.65, 1.47,
    1.355, 1.315, 1.255,
];
// FIG 4.2.3.1-27 ogive wave drag: X1 = (base/max diameter)^2, X2 = 2L/(beta D).
const T4227_X1: [f32; 12] = [
    0.0, 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.70, 0.80, 0.90, 1.0,
];
const T4227_X2: [f32; 10] = [1., 2., 3., 4., 5., 6., 8., 10., 16., 20.];
#[rustfmt::skip]
const D4227: [f32; 120] = [
    4.68, 3.55, 2.41, 1.63, 1.05, 0.63, 0.38, 0.20, 0.14, 0.05, 0.00, 0.0,
    4.68, 3.72, 2.73, 1.96, 1.39, 0.91, 0.58, 0.33, 0.25, 0.09, 0.01, 0.0,
    4.68, 3.81, 2.90, 2.17, 1.56, 1.06, 0.68, 0.40, 0.30, 0.13, 0.02, 0.0,
    4.68, 3.89, 3.01, 2.30, 1.70, 1.19, 0.78, 0.45, 0.34, 0.16, 0.03, 0.0,
    4.68, 3.93, 3.15, 2.42, 1.80, 1.28, 0.85, 0.50, 0.38, 0.18, 0.04, 0.0,
    4.68, 3.97, 3.25, 2.54, 1.90, 1.35, 0.90, 0.55, 0.41, 0.20, 0.05, 0.0,
    4.68, 4.06, 3.36, 2.69, 2.04, 1.47, 1.00, 0.60, 0.45, 0.22, 0.06, 0.0,
    4.68, 4.14, 3.47, 2.80, 2.15, 1.57, 1.06, 0.65, 0.49, 0.24, 0.07, 0.0,
    4.68, 4.22, 3.65, 3.00, 2.32, 1.71, 1.17, 0.73, 0.55, 0.28, 0.08, 0.0,
    4.68, 4.34, 3.80, 3.16, 2.48, 1.82, 1.25, 0.80, 0.60, 0.30, 0.09, 0.0,
];
// FIG 4.2.3.1-28 cone wave drag (same arguments).
const T4228_X1: [f32; 10] = [0.0, 0.05, 0.15, 0.30, 0.40, 0.50, 0.60, 0.75, 0.85, 1.0];
const T4228_X2: [f32; 10] = [1., 2., 3., 4., 5., 6., 8., 10., 15., 20.];
#[rustfmt::skip]
const D4228: [f32; 100] = [
    0.45, 0.58, 0.55, 0.41, 0.30, 0.18, 0.11, 0.060, 0.030, 0.0,
    1.75, 1.50, 1.18, 0.80, 0.58, 0.40, 0.25, 0.065, 0.035, 0.0,
    2.49, 2.00, 1.52, 1.00, 0.70, 0.48, 0.30, 0.070, 0.040, 0.0,
    3.00, 2.35, 1.78, 1.15, 0.82, 0.56, 0.34, 0.075, 0.045, 0.0,
    3.35, 2.65, 1.95, 1.25, 0.90, 0.61, 0.39, 0.080, 0.055, 0.0,
    3.72, 2.93, 2.10, 1.36, 1.00, 0.67, 0.42, 0.085, 0.060, 0.0,
    4.25, 3.25, 2.39, 1.50, 1.08, 0.73, 0.48, 0.095, 0.065, 0.0,
    4.90, 3.55, 2.55, 1.62, 1.14, 0.80, 0.50, 0.100, 0.075, 0.0,
    5.10, 4.10, 2.90, 1.88, 1.35, 0.90, 0.58, 0.190, 0.085, 0.0,
    5.50, 4.45, 3.12, 2.01, 1.43, 0.98, 0.62, 0.250, 0.100, 0.0,
];
// FIG 4.2.3.1-44A afterbody interference: X1 = (DB/DA)^2, X2 = LN/L.
const T44A_X1: [f32; 6] = [0.0, 0.05, 0.10, 0.20, 0.4, 1.0];
const T44A_X2: [f32; 3] = [0.5, 1., 2.];
#[rustfmt::skip]
const D44A: [f32; 18] = [
    2.40, 2.12, 1.94, 1.63, 1.16, 0.0,
    1.30, 1.18, 1.08, 0.90, 0.65, 0.0,
    0.64, 0.58, 0.53, 0.44, 0.31, 0.0,
];
// FIG 4.2.3.1-60 base drag of a cylindrical afterbody vs Mach.
const T4360: [f32; 13] = [
    1.00, 1.125, 1.25, 1.50, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0,
];
const D4360: [f32; 13] = [
    0.178, 0.215, 0.20, 0.178, 0.144, 0.118, 0.097, 0.080, 0.068, 0.057, 0.049, 0.042, 0.037,
];
// FIGs 4.2.3.1-50/55 base pressure: X1 = Mach, X2 = D2/DMAX, X3 = LBT/D1.
const T5055_X1: [f32; 11] = [1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0];
const T5055_X2: [f32; 5] = [0.2, 0.4, 0.6, 0.8, 1.0];
const T5055_X3: [f32; 5] = [2.0, 3.0, 4.0, 5.0, 6.0];
// FIG 4.2.3.1-50 A-E (ogive boat tails).
#[rustfmt::skip]
const D4350: [f32; 275] = [
    -0.055, -0.055, -0.055, -0.055, -0.054, -0.052, -0.048, -0.042, -0.037, -0.031, -0.025,
    -0.055, -0.055, -0.055, -0.055, -0.054, -0.052, -0.048, -0.042, -0.037, -0.031, -0.025,
    -0.055, -0.055, -0.055, -0.055, -0.054, -0.052, -0.048, -0.042, -0.037, -0.031, -0.025,
    -0.118, -0.118, -0.118, -0.103, -0.088, -0.075, -0.064, -0.054, -0.046, -0.038, -0.032,
    -0.211, -0.182, -0.147, -0.118, -0.097, -0.081, -0.069, -0.059, -0.050, -0.042, -0.035,
    -0.005, -0.005, -0.005, -0.005, -0.005, -0.005, -0.005, -0.005, -0.005, -0.006, -0.006,
    -0.031, -0.031, -0.031, -0.031, -0.031, -0.031, -0.031, -0.028, -0.026, -0.024, -0.022,
    -0.069, -0.069, -0.069, -0.069, -0.067, -0.058, -0.050, -0.043, -0.037, -0.032, -0.026,
    -0.111, -0.111, -0.111, -0.105, -0.089, -0.073, -0.061, -0.051, -0.044, -0.037, -0.030,
    -0.210, -0.180, -0.145, -0.118, -0.096, -0.080, -0.068, -0.058, -0.050, -0.042, -0.036,
    -0.024, -0.024, -0.024, -0.024, -0.024, -0.024, -0.024, -0.023, -0.021, -0.017, -0.012,
    -0.044, -0.044, -0.044, -0.044, -0.044, -0.043, -0.040, -0.036, -0.032, -0.028, -0.024,
    -0.075, -0.075, -0.075, -0.075, -0.071, -0.062, -0.054, -0.046, -0.040, -0.035, -0.030,
    -0.112, -0.112, -0.111, -0.101, -0.087, -0.074, -0.062, -0.052, -0.045, -0.038, -0.032,
    -0.210, -0.180, -0.144, -0.117, -0.097, -0.082, -0.069, -0.059, -0.050, -0.042, -0.035,
    -0.030, -0.030, -0.030, -0.030, -0.030, -0.030, -0.030, -0.031, -0.029, -0.026, -0.023,
    -0.050, -0.050, -0.050, -0.050, -0.052, -0.050, -0.046, -0.040, -0.035, -0.031, -0.027,
    -0.080, -0.080, -0.080, -0.081, -0.074, -0.065, -0.056, -0.048, -0.041, -0.035, -0.030,
    -0.112, -0.112, -0.112, -0.102, -0.087, -0.074, -0.062, -0.053, -0.045, -0.038, -0.033,
    -0.210, -0.140, -0.146, -0.118, -0.098, -0.082, -0.069, -0.058, -0.050, -0.043, -0.036,
    -0.033, -0.033, -0.033, -0.033, -0.033, -0.033, -0.033, -0.032, -0.029, -0.026, -0.024,
    -0.058, -0.058, -0.058, -0.058, -0.058, -0.054, -0.048, -0.042, -0.037, -0.033, -0.029,
    -0.084, -0.084, -0.084, -0.084, -0.074, -0.065, -0.056, -0.049, -0.042, -0.036, -0.031,
    -0.116, -0.116, -0.116, -0.104, -0.088, -0.074, -0.062, -0.052, -0.045, -0.039, -0.033,
    -0.211, -0.180, -0.146, -0.118, -0.097, -0.081, -0.069, -0.058, -0.050, -0.042, -0.036,
];
// FIG 4.2.3.1-55 A-E (cone boat tails).
#[rustfmt::skip]
const D4355: [f32; 275] = [
    -0.026, -0.026, -0.026, -0.026, -0.026, -0.026, -0.027, -0.025, -0.023, -0.021, -0.019,
    -0.042, -0.042, -0.042, -0.042, -0.042, -0.042, -0.038, -0.033, -0.027, -0.021, -0.016,
    -0.065, -0.065, -0.065, -0.065, -0.064, -0.058, -0.051, -0.043, -0.035, -0.028, -0.020,
    -0.118, -0.118, -0.118, -0.100, -0.083, -0.070, -0.060, -0.051, -0.043, -0.036, -0.030,
    -0.211, -0.180, -0.144, -0.118, -0.097, -0.081, -0.068, -0.057, -0.049, -0.041, -0.035,
    -0.030, -0.030, -0.030, -0.030, -0.030, -0.030, -0.030, -0.030, -0.030, -0.030, -0.030,
    -0.046, -0.046, -0.046, -0.046, -0.046, -0.045, -0.042, -0.038, -0.035, -0.033, -0.030,
    -0.070, -0.070, -0.070, -0.070, -0.068, -0.060, -0.053, -0.046, -0.040, -0.036, -0.032,
    -0.117, -0.117, -0.117, -0.101, -0.085, -0.072, -0.061, -0.052, -0.044, -0.040, -0.034,
    -0.210, -0.180, -0.144, -0.118, -0.097, -0.081, -0.068, -0.058, -0.049, -0.042, -0.035,
    -0.032, -0.032, -0.032, -0.032, -0.032, -0.032, -0.032, -0.032, -0.030, -0.023, -0.017,
    -0.050, -0.050, -0.050, -0.050, -0.050, -0.050, -0.047, -0.042, -0.037, -0.033, -0.030,
    -0.074, -0.074, -0.074, -0.074, -0.071, -0.063, -0.055, -0.048, -0.041, -0.036, -0.032,
    -0.106, -0.106, -0.107, -0.099, -0.086, -0.074, -0.063, -0.053, -0.045, -0.040, -0.036,
    -0.210, -0.180, -0.144, -0.118, -0.098, -0.082, -0.069, -0.059, -0.050, -0.043, -0.037,
    -0.038, -0.038, -0.038, -0.038, -0.038, -0.038, -0.037, -0.034, -0.032, -0.030, -0.028,
    -0.060, -0.060, -0.060, -0.060, -0.059, -0.054, -0.048, -0.042, -0.038, -0.034, -0.030,
    -0.079, -0.079, -0.079, -0.078, -0.073, -0.064, -0.055, -0.048, -0.041, -0.036, -0.031,
    -0.108, -0.108, -0.108, -0.100, -0.085, -0.073, -0.062, -0.053, -0.045, -0.039, -0.033,
    -0.210, -0.180, -0.145, -0.118, -0.097, -0.082, -0.068, -0.059, -0.050, -0.043, -0.036,
    -0.050, -0.050, -0.050, -0.050, -0.049, -0.046, -0.042, -0.038, -0.033, -0.029, -0.027,
    -0.068, -0.068, -0.068, -0.068, -0.064, -0.057, -0.050, -0.044, -0.038, -0.033, -0.030,
    -0.084, -0.084, -0.084, -0.083, -0.076, -0.067, -0.058, -0.049, -0.042, -0.036, -0.032,
    -0.106, -0.106, -0.106, -0.101, -0.087, -0.074, -0.063, -0.053, -0.045, -0.038, -0.034,
    -0.211, -0.180, -0.146, -0.118, -0.097, -0.081, -0.068, -0.058, -0.049, -0.042, -0.035,
];
// HYPBOD FIG 4.2.1.1-24 ogive-nose Newtonian CNA: X1 = LN/DN, X2 = DS/DN.
const T425_X1: [f32; 8] = [1., 1.5, 2., 3., 4., 5., 6., 7.];
const T425_X2: [f32; 9] = [0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8];
#[rustfmt::skip]
const D425: [f32; 72] = [
    1.6, 1.766, 1.850, 1.930, 1.960, 1.976, 1.987, 1.990,
    1.6, 1.764, 1.850, 1.921, 1.950, 1.968, 1.975, 1.979,
    1.592, 1.758, 1.840, 1.904, 1.930, 1.942, 1.950, 1.950,
    1.581, 1.740, 1.810, 1.864, 1.889, 1.900, 1.901, 1.900,
    1.560, 1.704, 1.761, 1.809, 1.822, 1.830, 1.835, 1.838,
    1.530, 1.650, 1.693, 1.729, 1.739, 1.743, 1.750, 1.750,
    1.488, 1.565, 1.604, 1.629, 1.634, 1.640, 1.640, 1.642,
    1.419, 1.463, 1.489, 1.506, 1.510, 1.513, 1.516, 1.515,
    1.320, 1.332, 1.342, 1.354, 1.360, 1.365, 1.369, 1.370,
];
// HYPBOD FIG 4.2.2.1-27 ogive-nose XCP/LN (same arguments, X1 to 9).
const T422_X1: [f32; 9] = [1., 2., 3., 4., 5., 6., 7., 8., 9.];
const T422_X2: [f32; 9] = [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8];
#[rustfmt::skip]
const D422: [f32; 81] = [
    0.496, 0.470, 0.468, 0.468, 0.467, 0.467, 0.467, 0.467, 0.467,
    0.486, 0.457, 0.453, 0.451, 0.450, 0.448, 0.447, 0.446, 0.445,
    0.475, 0.441, 0.436, 0.433, 0.430, 0.428, 0.426, 0.425, 0.424,
    0.462, 0.426, 0.419, 0.414, 0.410, 0.407, 0.405, 0.403, 0.401,
    0.450, 0.408, 0.400, 0.394, 0.389, 0.385, 0.382, 0.380, 0.377,
    0.438, 0.390, 0.379, 0.371, 0.365, 0.360, 0.356, 0.353, 0.349,
    0.423, 0.367, 0.353, 0.344, 0.336, 0.329, 0.324, 0.320, 0.317,
    0.405, 0.337, 0.317, 0.306, 0.297, 0.291, 0.286, 0.282, 0.278,
    0.381, 0.296, 0.274, 0.260, 0.249, 0.242, 0.236, 0.231, 0.227,
];

/// Elliptic cross-section corrections to the potential (CNOCNS) and viscous
/// (CNOCNN) normal force, shared by BODYJM and SYPBOD; both are 1 for a
/// circle. ELLIP is the width/depth ratio.
pub(super) fn ellipse_factors(ellip: f32) -> (f32, f32) {
    let aob = if ellip < 1. { 1. / ellip } else { ellip };
    let cnocns = if ellip >= 1. { 1. / aob } else { aob };
    let a2b = aob * aob;
    let cnocnn = if ellip < 1. {
        let q = 1. - 1. / a2b;
        1.5 * aob.sqrt() * (-1. / a2b / powr(q, 1.5) * (aob * (1. + q.sqrt())).ln() + 1. / q)
    } else if ellip > 1. {
        1.5 * (1. / aob).sqrt()
            * (a2b / powr(a2b - 1., 1.5) * (a2b - 1.).sqrt().atan() - 1. / (a2b - 1.))
    } else {
        1.0
    };
    (cnocns, cnocnn)
}

/// Per-alpha body-axis and stability-axis coefficients from SYPBOD/HYPBOD,
/// in their BODY slots: CD (1..), CL (21..), CM (41..), CN (61..), CA (81..).
#[derive(Debug, Clone, Default)]
pub(super) struct Curves {
    pub cd: Vec<f32>,
    pub cl: Vec<f32>,
    pub cm: Vec<f32>,
    pub cn: Vec<f32>,
    pub ca: Vec<f32>,
}

/// SYPBOD work values (COMMON /SUPBOD/ SBD(n) and BD(536..635)). Lengths are
/// in the case's units; `UNUSED` marks slots the original leaves unset for
/// this body shape. Per-alpha vectors are empty for transonic points.
#[derive(Debug, Clone)]
pub struct SupersonicBody {
    /// SBD(1)-(3): nose + afterbody length, body length, tail length.
    pub nose_afterbody_length: f32,
    pub length: f32,
    pub tail_length: f32,
    /// SBD(4)-(6): nose, afterbody and base diameters (DN, D1, D2).
    pub nose_diameter: f32,
    pub afterbody_diameter: f32,
    pub base_diameter: f32,
    /// SBD(7): beta (0.98 for the transonic Mach 1.4 evaluation).
    pub beta: f32,
    /// SBD(8)-(10): afterbody, tail and nose fineness ratios.
    pub afterbody_fineness: f32,
    pub tail_fineness: f32,
    pub nose_fineness: f32,
    /// SBD(11)/(12): nose-cylinder XCP/LBP and its CMA per CNA.
    pub nose_xcp: f32,
    pub nose_cylinder_cma: f32,
    /// SBD(13)/(15): tail CMA and CNA increments.
    pub tail_cma: f32,
    pub tail_cna: f32,
    /// SBD(14)/(16) (radians): boat-tail and flare angles.
    pub boattail_angle: f32,
    pub flare_angle: f32,
    /// SBD(17)/(18): nose-cylinder CNA (per radian, own area) and body CNA
    /// (per degree, reference area).
    pub nose_cylinder_cna: f32,
    pub cna: f32,
    /// SBD(19)/(20): base and planform areas.
    pub base_area: f32,
    pub planform_area: f32,
    /// SBD(41..)/(61..)/(81..): crossflow Mach, crossflow drag coefficient
    /// and viscous crossflow normal force.
    pub crossflow_mach: Vec<f32>,
    pub crossflow_drag: Vec<f32>,
    pub crossflow: Vec<f32>,
    /// SBD(101): boat-tail XCP / tail length.
    pub boattail_xcp: f32,
    /// SBD(102): flare angle again, set only on the CNA flare branch.
    pub flare_angle_cna: f32,
    /// SBD(103): flare moment factor CMAP.
    pub flare_cmap: f32,
    /// SBD(105)/(106): planform centroid and body volume.
    pub planform_centroid: f32,
    pub volume: f32,
    /// SBD(107)/(108): nose wave drag factor and coefficient.
    pub nose_wave_factor: f32,
    pub nose_wave_drag: f32,
    /// SBD(110): CMA per degree.
    pub cma: f32,
    /// SBD(111)-(115): wetted area, Reynolds number used, cutoff Reynolds
    /// number, skin-friction and friction-drag coefficients.
    pub wetted_area: f32,
    pub reynolds: f32,
    pub cutoff_reynolds: f32,
    pub skin_friction: f32,
    pub friction_drag: f32,
    /// SBD(116)/(117): afterbody interference factor and drag.
    pub interference_factor: f32,
    pub interference_drag: f32,
    /// SBD(118)/(119): afterbody wave drag factor and coefficient.
    pub afterbody_wave_factor: f32,
    pub afterbody_wave_drag: f32,
    /// SBD(120)-(123): maximum diameter, base drag factor, base pressure
    /// coefficient and base drag.
    pub maximum_diameter: f32,
    pub base_drag_factor: f32,
    pub base_pressure: f32,
    pub base_drag: f32,
    /// SBD(124): zero-lift drag.
    pub zero_lift_drag: f32,
    /// SBD(135): tail (boat-tail or flare) angle.
    pub tail_angle: f32,
    /// BD(536..): equivalent radii, then potential/viscous CN and CM per alpha.
    pub equivalent_radius: Vec<f32>,
    pub cn_potential: Vec<f32>,
    pub cn_viscous: Vec<f32>,
    pub cm_potential: Vec<f32>,
    pub cm_viscous: Vec<f32>,
}

/// SYPBOD for one point. Transonic points use Mach 1.4 and stop after the
/// slopes and zero-lift drag, returning empty curves.
pub(super) fn sypbod(inp: &Inputs, point: &FlightPoint) -> Result<(SupersonicBody, Curves)> {
    let transonic = point.regime == Regime::Transonic;
    let (x, r, s, p) = (inp.x, inp.r, inp.s, inp.p);
    let (sr, crbar, xcg) = (inp.sref, inp.cbarr, inp.xcg);
    let n = x.len();
    let (rln, rla) = (inp.bln, inp.bla);
    let rlbp = rln + rla;
    let rlb = x[n - 1];
    // BTAIL = 1 (cone) or 2 (ogive) declares a tail; LOOP picks which
    // stations (nose end, afterbody end, base) define DN, D1 and D2.
    let tail = inp.btail == 1. || inp.btail == 2.;
    let rlbt = if tail { rlb - rlbp } else { 0.0 };
    let nostal = tail && rla == 0.0;
    let stations = if tail && rla != 0.0 {
        3
    } else if !tail && rla == 0.0 {
        1
    } else {
        2
    };
    let diameter = |at: f32| -> Result<f32> { Ok(interx(&[x], r, &[at], &[(0, 0)])? * 2.) };
    let dn = diameter(rln)?;
    let (mut d1, mut d2) = (dn, dn);
    if stations >= 2 {
        if nostal {
            d2 = diameter(rlb)?;
        } else {
            d1 = diameter(rlbp)?;
            if stations == 2 {
                d2 = d1;
            }
        }
    }
    if stations == 3 {
        d2 = diameter(rlb)?;
    }
    let (mach, beta) = if transonic {
        (1.4, 0.98)
    } else {
        (point.mach, (point.mach * point.mach - 1.).sqrt())
    };

    // Lift-curve slope: nose-cylinder figure plus boat-tail/flare increment.
    let dcyl = (dn + d1) / 2.0;
    if d2 == 0.0 {
        d2 = 0.3 * dcyl;
    }
    let fa = rla / dcyl;
    let fn_ = rln / dn;
    let at = [beta / fn_, fa / fn_];
    let cone_nose = inp.bnose == 1.;
    let cnaoc = if cone_nose {
        interx(&[&T4221B_X1, &T4221B_X2], &D4221B, &at, &[(1, 1), (0, 2)])?
    } else {
        interx(&[&T4221A_X1, &T4221A_X2], &D4221A, &at, &[(1, 2), (0, 2)])?
    };
    let (mut thetab, mut thetaf, mut thetaf_cna, mut thetat, mut fb) =
        (UNUSED, UNUSED, UNUSED, UNUSED, UNUSED);
    let delcna = if !tail {
        0.0
    } else if d2 / d1 > 1. {
        let dccos = interx(&[&T4222B], &D4222B, &[d2 / d1], &[(0, 2)])?;
        thetaf = (0.5 * (d2 - d1) / rlbt).atan();
        thetaf_cna = thetaf;
        thetat = thetaf;
        let c = thetaf.cos();
        dccos * (c * c)
    } else {
        thetab = (0.5 * (d1 - d2) / rlbt).atan();
        thetat = thetab;
        fb = rlbt / d1;
        let dcd1d2 = interx(&[&T4222A], &D4222A, &[fb / beta], &[(0, 0)])?;
        let q = d2 / d1;
        dcd1d2 * (1. - q * q)
    };
    let cna = (cnaoc + delcna) * PI * (d1 * d1) / (4. * RAD * sr);

    // Supersonic lift vs alpha: slender-body potential plus crossflow drag.
    let na = inp.alphas.len();
    let mut o = SupersonicBody {
        nose_afterbody_length: rlbp,
        length: rlb,
        tail_length: rlbt,
        nose_diameter: dn,
        afterbody_diameter: d1,
        base_diameter: d2,
        beta,
        afterbody_fineness: fa,
        tail_fineness: fb,
        nose_fineness: fn_,
        nose_xcp: UNUSED,
        nose_cylinder_cma: UNUSED,
        tail_cma: UNUSED,
        tail_cna: delcna,
        boattail_angle: thetab,
        flare_angle: thetaf,
        nose_cylinder_cna: cnaoc,
        cna,
        base_area: UNUSED,
        planform_area: UNUSED,
        crossflow_mach: Vec::new(),
        crossflow_drag: Vec::new(),
        crossflow: Vec::new(),
        boattail_xcp: UNUSED,
        flare_angle_cna: thetaf_cna,
        flare_cmap: UNUSED,
        planform_centroid: UNUSED,
        volume: UNUSED,
        nose_wave_factor: UNUSED,
        nose_wave_drag: UNUSED,
        cma: UNUSED,
        wetted_area: UNUSED,
        reynolds: UNUSED,
        cutoff_reynolds: UNUSED,
        skin_friction: UNUSED,
        friction_drag: UNUSED,
        interference_factor: UNUSED,
        interference_drag: UNUSED,
        afterbody_wave_factor: UNUSED,
        afterbody_wave_drag: UNUSED,
        maximum_diameter: UNUSED,
        base_drag_factor: UNUSED,
        base_pressure: UNUSED,
        base_drag: UNUSED,
        zero_lift_drag: UNUSED,
        tail_angle: thetat,
        equivalent_radius: Vec::new(),
        cn_potential: Vec::new(),
        cn_viscous: Vec::new(),
        cm_potential: Vec::new(),
        cm_viscous: Vec::new(),
    };
    let mut curves = Curves::default();
    // SYPBOD sets ELLIP = 1 when it was not input (a circular section).
    let ellip = if inp.ellip <= UNUSED { 1.0 } else { inp.ellip };
    let (cnocns, cnocnn) = ellipse_factors(ellip);
    let sb = PI * (d2 * d2) / 4.;
    let mut sp = UNUSED;
    if !transonic {
        let a1 = if ellip == 1.0 {
            cna * sr * RAD / 2.0
        } else {
            sb
        };
        sp = last(x, r)? * 2.;
        for &alpha in inp.alphas {
            let a = alpha / RAD;
            let mc = mach * a.sin().abs();
            let cdc = interx(&[&T4217B], &D4217B, &[mc], &[(0, 0)])?;
            let sa = a.sin();
            let mut cflow = cdc * sp * (sa * sa) / sr;
            if a < 0.0 {
                cflow = -cflow;
            }
            let cnpot = (2. * a).sin() * (a / 2.).cos() * a1 * cnocns / sr;
            let cnvis = cflow * cnocnn;
            o.crossflow_mach.push(mc);
            o.crossflow_drag.push(cdc);
            o.crossflow.push(cflow);
            o.cn_potential.push(cnpot);
            o.cn_viscous.push(cnvis);
            curves.cn.push(cnpot + cnvis);
        }
        o.base_area = sb;
        o.planform_area = sp;
    }

    // Pitching-moment slope: nose-cylinder XCP figure (left side for
    // beta/FN <= 1, else right side at FN/beta) plus the tail increment.
    let (v1, v2) = (beta / fn_, fa / fn_);
    let (v1, right) = if v1 > 1. {
        (1. / v1, true)
    } else {
        (v1, false)
    };
    let xcplb = match (cone_nose, right) {
        (false, false) => interx(
            &[&T4218A_X1, &T4218A_X2],
            &DL218A,
            &[v1, v2],
            &[(1, 0), (0, 2)],
        )?,
        (false, true) => interx(
            &[&T4218A_X1, &T4218A_X2],
            &DR218A,
            &[v1, v2],
            &[(1, 0), (0, 2)],
        )?,
        (true, false) => interx(
            &[&T4218B_X1, &T4218B_X2],
            &DL218B,
            &[v1, v2],
            &[(1, 0), (0, 2)],
        )?,
        (true, true) => interx(
            &[&T4218B_X1, &T4218B_X2],
            &DR218B,
            &[v1, v2],
            &[(1, 0), (0, 2)],
        )?,
    };
    let cmaoc = (xcg / rlbp - xcplb) * cnaoc;
    // The moment uses the flare branch when D2 = D1, where the CNA branch
    // took the boat tail: the flare angle is then still UNUSED (~0 deg).
    let delcma = if !tail {
        0.0
    } else if d2 / d1 >= 1.0 {
        let cmap = interx(
            &[&T4220A_X1, &T4220A_X2],
            &D4220A,
            &[thetaf * RAD, d1 / d2],
            &[(0, 0), (0, 0)],
        )?;
        o.flare_cmap = cmap;
        cmap * (d2 * d2 * d2 / ((d1 * d1) * rlbp)) + ((xcg - rlbp) / rlbp) * delcna
    } else {
        let xcpblb = interx(
            &[&T42119_X1, &T42119_X2],
            &D42119,
            &[beta * thetab.tan(), d2 / d1],
            &[(0, 2), (0, 0)],
        )?;
        o.boattail_xcp = xcpblb;
        delcna * (2. * xcg / rlbp - 1.0 - xcpblb * rlbt / rlbp)
    };
    let cma = (cmaoc + delcma) * PI * (d1 * d1) * rlbp / (4. * sr * crbar * RAD);
    o.nose_xcp = xcplb;
    o.nose_cylinder_cma = cmaoc;
    o.tail_cma = delcma;
    o.cma = cma;

    if !transonic {
        // Planform centroid, volume, then potential + viscous moment vs alpha.
        let req: Vec<f32> = s.iter().map(|&sk| (sk / PI).sqrt()).collect();
        let rx: Vec<f32> = req.iter().zip(x).map(|(q, xk)| q * xk).collect();
        let xc = 2. * last(x, &rx)? / sp;
        let vb = frustum_volume(x, &req)?;
        let arg1 = if ellip == 1.0 {
            cma * RAD / 2.0
        } else {
            vb / (sr * crbar) - (rlb - xcg) * sb / (sr * crbar)
        };
        for (j, &alpha) in inp.alphas.iter().enumerate() {
            let a = alpha / RAD;
            let cflow = o.crossflow[j];
            let cmpot = (2. * a).sin() * (a / 2.).cos() * arg1 * cnocns;
            let cmvis = cflow / crbar * (xcg - xc) * cnocnn;
            o.cm_potential.push(cmpot);
            o.cm_viscous.push(cmvis);
            curves.cm.push(cmvis + cmpot);
        }
        o.equivalent_radius = req;
        o.planform_centroid = xc;
        o.volume = vb;
    }

    // Nose wave drag (FIG 4.2.3.1-27 ogive / -28 cone at zero base ratio).
    let at = [0.0, 2. * rln / (beta * dn)];
    let cdn2p = if cone_nose {
        interx(&[&T4228_X1, &T4228_X2], &D4228, &at, &[(0, 0), (0, 2)])?
    } else {
        interx(&[&T4227_X1, &T4227_X2], &D4227, &at, &[(0, 0), (2, 2)])?
    };
    let dn2 = dn * dn;
    let cdn2 = cdn2p * PI * (dn2 * dn2) / (16. * sr * (rln * rln));
    o.nose_wave_factor = cdn2p;
    o.nose_wave_drag = cdn2;

    // Skin friction: trapezoidal wetted area, roughness-limited Reynolds
    // number (FIG 4.1.5.1-27, Mach capped at 3) and FIG26.
    let mut ss = 0.0;
    for j in 0..n - 1 {
        ss += (p[j + 1] + p[j]) * (x[j + 1] - x[j]) / 2.;
    }
    let mut rnb = rlb * point.reynolds;
    if inp.roughness == 0.0 {
        // The original then reads an unassigned Mach for FIG26.
        return Err(AeroError::Invalid(
            "supersonic body drag needs a nonzero ROUGFC".into(),
        ));
    }
    let arg = 12. * rlb / inp.roughness;
    let rach = if mach > 3.0 { 3.0 } else { mach };
    let hold = Limits::default();
    let cept = interp::tbfunx(&X27M, &X27I, rach, hold)?.value;
    let rlcoff = powr(arg, 1.0482) * powr(10.0, cept);
    if rlcoff < rnb {
        rnb = rlcoff;
    }
    let cf = super::body::fig26(rnb, rach);
    let cdf = cf * ss / sr;
    o.wetted_area = ss;
    o.reynolds = rnb;
    o.cutoff_reynolds = rlcoff;
    o.skin_friction = cf;
    o.friction_drag = cdf;

    // Afterbody interference drag (nose-afterbody, else afterbody-tail joint).
    let joint = if rla != 0.0 {
        Some((d1, dn, rla))
    } else if rlbt != 0.0 {
        Some((d1, d2, rlbt))
    } else {
        None
    };
    let cdanc = match joint {
        Some((da, db, length)) => {
            let q = db / da;
            let cdanf = interx(
                &[&T44A_X1, &T44A_X2],
                &D44A,
                &[q * q, rln / length],
                &[(0, 0), (0, 0)],
            )?;
            o.interference_factor = cdanf;
            let db2 = db * db;
            cdanf * PI * (db2 * db2) / (16. * sr * (length * length))
        }
        None => 0.0,
    };
    o.interference_drag = cdanc;

    // Afterbody (tail) wave drag; CDA stays UNUSED without one, so CDO then
    // carries an extra 1E-30 like the original.
    let mut cda = UNUSED;
    if rla == 0.0 || !tail {
        o.afterbody_wave_factor = 0.0;
    } else {
        let (dd, aa) = if d2 / d1 > 1. { (d2, d1) } else { (d1, d2) };
        let q = aa / dd;
        let at = [q * q, 2. * rlbt / (beta * dd)];
        let cdab = if inp.btail == 1. {
            interx(&[&T4228_X1, &T4228_X2], &D4228, &at, &[(0, 0), (0, 2)])?
        } else {
            interx(&[&T4227_X1, &T4227_X2], &D4227, &at, &[(0, 0), (2, 2)])?
        };
        o.afterbody_wave_factor = cdab;
        let dd2 = dd * dd;
        cda = cdab * PI * (dd2 * dd2) / (16. * sr * (rlbt * rlbt));
    }
    o.afterbody_wave_drag = cda;

    // Base drag: FIG 4.2.3.1-60 for a cylindrical end, else the boat-tail
    // base-pressure figures 50 (ogive) / 55 (cone).
    let mut dmax = r[0];
    for &rk in r {
        if dmax < rk {
            dmax = rk;
        }
    }
    let dmax = 2. * dmax;
    let cdd = if rlbt != 0.0 || d2 / d1 < 1.0 {
        let table = if inp.btail == 1. { &D4355 } else { &D4350 };
        let cpb = interx(
            &[&T5055_X1, &T5055_X2, &T5055_X3],
            table,
            &[mach, d2 / dmax, rlbt / d1],
            &[(0, 0), (0, 0), (2, 2)],
        )?;
        o.base_pressure = cpb;
        let q = d2 / dmax;
        -cpb * (q * q)
    } else {
        interx(&[&T4360], &D4360, &[mach], &[(0, 0)])?
    };
    let cdb = cdd * PI * (dmax * dmax) / (4. * sr);
    let cdo = cdf + cdn2 + cda + cdanc + cdb;
    o.maximum_diameter = dmax;
    o.base_drag_factor = cdd;
    o.base_drag = cdb;
    o.zero_lift_drag = cdo;
    finite(&[o.cna, o.cma, o.zero_lift_drag])?;
    if transonic {
        return Ok((o, curves));
    }

    // Body-axis CA from CDO, then stability-axis CL/CD.
    for (j, &alpha) in inp.alphas.iter().enumerate() {
        let (sina, cosa) = ((alpha / RAD).sin(), (alpha / RAD).cos());
        let ca = cdo * (cosa * cosa);
        let cn = curves.cn[j];
        curves.ca.push(ca);
        curves.cl.push(cn * cosa - ca * sina);
        curves.cd.push(ca * cosa + cn * sina);
    }
    debug_assert_eq!(curves.cd.len(), na);
    Ok((o, curves))
}

/// HYPBOD work values (SBD(n)); `UNUSED` marks slots left unset.
#[derive(Debug, Clone)]
pub struct HypersonicBody {
    /// SBD(1)-(6): nose + afterbody length, body length, tail length, nose,
    /// afterbody and base diameters.
    pub nose_afterbody_length: f32,
    pub length: f32,
    pub tail_length: f32,
    pub nose_diameter: f32,
    pub afterbody_diameter: f32,
    pub base_diameter: f32,
    /// SBD(10): nose fineness ratio.
    pub nose_fineness: f32,
    /// SBD(14): tail angle when the tail is not a flare (else UNUSED).
    pub boattail_angle: f32,
    /// SBD(16)/(102): flare angle (radians), flares only.
    pub flare_angle: f32,
    /// SBD(18)/(110): body CNA and CMA per degree.
    pub cna: f32,
    pub cma: f32,
    /// SBD(125)-(129): nose CNA (own area, per radian), nose XCP/LN,
    /// nose half-angle, then nose CNA and CMA per degree.
    pub nose_cna_factor: f32,
    pub nose_xcp: f32,
    pub nose_angle: f32,
    pub nose_cna: f32,
    pub nose_cma: f32,
    /// SBD(130)-(134): afterbody flare angle, factors and slopes.
    pub afterbody_angle: f32,
    pub afterbody_cna_factor: f32,
    pub afterbody_cma_factor: f32,
    pub afterbody_cna: f32,
    pub afterbody_cma: f32,
    /// SBD(135)-(139): tail angle, factors and slopes.
    pub tail_angle: f32,
    pub tail_cna_factor: f32,
    pub tail_cma_factor: f32,
    pub tail_cna: f32,
    pub tail_cma: f32,
    /// SBD(140): modified-Newtonian pressure factor K.
    pub newtonian_factor: f32,
    /// SBD(141..)/(161..): surface slope (deg) and moment arm per station.
    pub surface_angle: Vec<f32>,
    pub moment_arm: Vec<f32>,
    /// SBD(181..)/(201..): normal-force and moment integrals per alpha.
    pub cn_integral: Vec<f32>,
    pub cm_integral: Vec<f32>,
}

/// HYPBOD: Newtonian slopes per segment and impact-theory coefficients.
pub(super) fn hypbod(inp: &Inputs, point: &FlightPoint) -> Result<(HypersonicBody, Curves)> {
    let (x, r) = (inp.x, inp.r);
    let (sr, cbar, xcg, ds) = (inp.sref, inp.cbarr, inp.xcg, inp.ds);
    let n = x.len();
    let mach = point.mach;
    let (rln, rla) = (inp.bln, inp.bla);
    let rlbp = rln + rla;
    let rlb = x[n - 1];
    let mut rlbt = rlb - rlbp;
    // A tail shorter than 1% of the body is ignored.
    let tail = (rlbt / rlb).abs() >= 0.01;
    if !tail {
        rlbt = 0.0;
    }
    let diameter = |at: f32| -> Result<f32> { Ok(2. * interx(&[x], r, &[at], &[(0, 0)])?) };
    let dn = diameter(rln)?;
    let d1 = if rla == 0.0 { dn } else { diameter(rlbp)? };
    let d2 = if tail { diameter(rlb)? } else { d1 };

    // Nose: ogive figures or closed-form cone (with bluntness DS) results.
    let fn_ = rln / dn;
    let thetan = (0.5 * (dn - ds) / (dn * fn_ - 0.5 * ds)).atan();
    let (cnanf, xcpln) = if inp.bnose == 1. {
        let c2 = thetan.cos() * thetan.cos();
        let q = ds / dn;
        let cnanf = 2. * c2 * (1. - 0.5 * (q * q) * c2);
        let arg1 = 2. / (3. * c2);
        let arg = ds * thetan.cos() / dn;
        let sint = thetan.sin();
        let arg2 = 1. - 1.5 * (1. - sint) * arg;
        let arg3 = 0.25 * (2. - 3. * sint) * (arg * arg * arg);
        let arg4 = 1. - 0.5 * (arg * arg);
        let arg5 = 1. - arg / (1. + sint);
        (cnanf, arg1 * ((arg2 + arg3) / (arg4 * arg5)))
    } else {
        let at = [rln / dn, ds / dn];
        let hold = [(0, 0), (0, 0)];
        (
            interx(&[&T425_X1, &T425_X2], &D425, &at, &hold)?,
            interx(&[&T422_X1, &T422_X2], &D422, &at, &hold)?,
        )
    };
    let ann = PI * (dn * dn) / 4.;
    let cnan = cnanf * ann / (RAD * sr);
    let cman = cnan * (xcg / rln - xcpln) * rln / cbar;

    // Newtonian frustum factors for a flared afterbody or tail; zero for
    // cylinders and boat tails.
    let frustum = |ratio: f32, theta: f32| {
        let c2 = theta.cos() * theta.cos();
        let cna = 2. * c2 * (1. - ratio * ratio);
        let arg1 = 3. * theta.tan();
        let arg2 = 2. * (1. - ratio * ratio * ratio);
        let arg3 = 3. * ratio * c2 * (1. - ratio * ratio);
        (cna, -(arg2 - arg3) / arg1)
    };
    let mut thetaa = UNUSED;
    let (cnaaf, cmaaf) = if rla == 0.0 {
        (0.0, 0.0)
    } else {
        let ratioa = dn / d1;
        thetaa = (0.5 * (d1 - dn) / rla).atan();
        if ratioa >= 1.0 {
            (0.0, 0.0)
        } else {
            frustum(ratioa, thetaa)
        }
    };
    let aaa = PI * (d1 * d1) / 4.;
    let cnaa = cnaaf * aaa / (RAD * sr);
    let cmaa = cmaaf * aaa * d1 / (cbar * sr) + cnaa * (xcg - rln) / cbar;
    let (mut thetat, mut boattail_angle, mut flare_angle) = (UNUSED, UNUSED, UNUSED);
    let mut flare = None;
    if tail {
        let ratiot = d2 / d1;
        thetat = (0.5 * (d2 - d1) / rlbt).atan();
        if ratiot > 1.0 {
            flare_angle = thetat;
            flare = Some(frustum(ratiot, thetat));
        }
    }
    let (cnatf, cmatf) = flare.unwrap_or_else(|| {
        boattail_angle = thetat;
        (0.0, 0.0)
    });
    let att = PI * (d1 * d1) / 4.;
    let cnat = cnatf * att / (RAD * sr);
    let cmat = cmatf * att * d2 / (cbar * RAD * sr) + cnat * (xcg - rln - rla) / cbar;
    let cma = cman + cmaa + cmat;
    let cna = cnan + cnaa + cnat;

    // Surface slope per station (the last repeats the previous one).
    let silent = Limits {
        lower: Extrapolation::HoldSilent,
        upper: Extrapolation::HoldSilent,
    };
    let mut theta = Vec::with_capacity(n);
    let mut lx = Vec::with_capacity(n);
    for k in 0..n - 1 {
        let slope = interp::tbfunx(x, r, x[k], silent)?.derivative;
        theta.push(RAD * slope.atan());
        lx.push(xcg - x[k]);
    }
    theta.push(theta[n - 2]);
    lx.push(xcg - x[n - 1]);
    let k = 1.833 * (1. - 0.4545 / (mach * mach));
    let factor = k * rlb / sr;

    // Newtonian pressure integrated around each station (wetted angle PHE)
    // and along the body, per alpha.
    let indep: Vec<f32> = x.iter().map(|&xk| xk / rlb).collect();
    let mut curves = Curves::default();
    let (mut intgcn, mut intgcm) = (Vec::new(), Vec::new());
    for &alpha in inp.alphas {
        let a = (alpha / RAD).abs();
        let (sa, ca, ta) = (a.sin(), a.cos(), a.tan());
        let mut dep = Vec::with_capacity(n);
        let mut dep1 = Vec::with_capacity(n);
        let mut dep2 = Vec::with_capacity(n);
        for i in 0..n {
            let t = theta[i] / RAD;
            let (tn, cnn, sn) = (t.tan(), t.cos(), t.sin());
            let phe = if (alpha / RAD).abs() > t.abs() {
                (tn / ta).acos()
            } else if t > 0.0 {
                0.0
            } else {
                PI
            };
            let (sp, cp) = (phe.sin(), phe.cos());
            let cs = cnn * sa;
            let arg1 = (2. / 3.) * (cs * cs) * sp * (cp * cp + 2.);
            let arg2 = 4. * sn * cnn * ca * sa * (PI / 2. - 0.5 * sp * cp - phe / 2.);
            let sc = sn * ca;
            let arg3 = 2. * (sc * sc) * sp;
            let ktheta = arg1 + arg2 + arg3;
            let cs2 = ca * sn;
            let arg6 = 2. * (cs2 * cs2) * tn * (PI - phe);
            let arg7 = 4. * ca * sa * sp * (sn * sn);
            let arg8 = cnn * sn * (sa * sa) * (PI - phe - sp * cp);
            let kaf = arg6 + arg7 + arg8;
            dep.push(ktheta * r[i]);
            dep1.push(dep[i] * lx[i]);
            dep2.push(kaf * r[i]);
        }
        let cn_int = last(&indep, &dep)?;
        let cm_int = last(&indep, &dep1)?;
        let ca_int = last(&indep, &dep2)?;
        let sign = if alpha < 0.0 { -1.0 } else { 1.0 };
        curves.cn.push(sign * factor * cn_int);
        curves.cm.push(sign * factor * cm_int / cbar);
        curves.ca.push(factor * ca_int);
        intgcn.push(cn_int);
        intgcm.push(cm_int);
    }
    // Stability axes from body-axis CN and CA.
    for (j, &alpha) in inp.alphas.iter().enumerate() {
        let a = alpha / RAD;
        let (cn, caf) = (curves.cn[j], curves.ca[j]);
        curves.cl.push(cn * a.cos() - caf * a.sin());
        curves.cd.push(caf * a.cos() + cn * a.sin());
    }
    finite(&[cna, cma])?;
    Ok((
        HypersonicBody {
            nose_afterbody_length: rlbp,
            length: rlb,
            tail_length: rlbt,
            nose_diameter: dn,
            afterbody_diameter: d1,
            base_diameter: d2,
            nose_fineness: fn_,
            boattail_angle,
            flare_angle,
            cna,
            cma,
            nose_cna_factor: cnanf,
            nose_xcp: xcpln,
            nose_angle: thetan,
            nose_cna: cnan,
            nose_cma: cman,
            afterbody_angle: thetaa,
            afterbody_cna_factor: cnaaf,
            afterbody_cma_factor: cmaaf,
            afterbody_cna: cnaa,
            afterbody_cma: cmaa,
            tail_angle: thetat,
            tail_cna_factor: cnatf,
            tail_cma_factor: cmatf,
            tail_cna: cnat,
            tail_cma: cmat,
            newtonian_factor: k,
            surface_angle: theta,
            moment_arm: lx,
            cn_integral: intgcn,
            cm_integral: intgcm,
        },
        curves,
    ))
}
