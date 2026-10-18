//! Flight conditions: the 1962 US Standard Atmosphere (port of ATMOS) and
//! the main program's LOOP 1/2/3 Mach/altitude sweep that sets VINF, MACH
//! and RNNUB before each MAIN00 call.
// Constants are copied digit-for-digit from the original DATA statements.
#![allow(clippy::excessive_precision)]

use crate::{
    input::{Case, UNUSED},
    math::{finite, powr, MathError},
};

/// Atmospheric state returned by ATMOS (English units).
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Atmosphere {
    /// Speed of sound, ft/s.
    pub sound_speed: f32,
    /// Pressure, lb/ft^2.
    pub pressure: f32,
    /// Density, slug/ft^3.
    pub density: f32,
    /// Temperature, deg R.
    pub temperature: f32,
}

const G0: f32 = 32.1740484;
const WM0: f32 = 28.9644;
const R0: f32 = 20890855.0;
const GMRS: f32 = 0.018743418;
const HG: [f32; 10] = [
    -16404., 0.0, 36089., 65617., 104987., 154199., 170604., 200131., 250186., 291160.,
];
const ZM: [f32; 14] = [
    295276., 328084., 360892., 393701., 492126., 524934., 557743., 623360., 754593., 984252.,
    1312336., 1640420., 1968504., 2296588.,
];
const WM: [f32; 14] = [
    28.9644, 28.88, 28.56, 28.07, 26.92, 26.66, 26.4, 25.85, 24.7, 22.66, 19.94, 17.94, 16.84,
    16.17,
];
const TM: [f32; 23] = [
    577.17, 518.67, 389.97, 389.97, 411.57, 487.17, 487.17, 454.77, 325.17, 325.17, 379.17, 469.17,
    649.17, 1729.17, 1999.17, 2179.17, 2431.17, 2791.17, 3295.17, 3889.17, 4357.17, 4663.17,
    4861.17,
];
const PM: [f32; 22] = [
    3711.0839,
    2116.2165,
    472.67563,
    114.34314,
    18.128355,
    2.3162178,
    1.2321972,
    3.8030279E-01,
    2.1671352E-02,
    3.4313478E-03,
    6.2773411E-04,
    1.5349091E-04,
    5.2624212E-05,
    1.0561806E-05,
    7.7083076E-06,
    5.8267151E-06,
    3.5159854E-06,
    1.4520255E-06,
    3.9290563E-07,
    8.4030242E-08,
    2.2835256E-08,
    7.1875452E-09,
];

/// Port of ATMOS: geopotential layers to 295276 ft, then geometric layers
/// with varying molecular weight (inverse-square gravity throughout).
/// Indices below are the Fortran ones minus one.
pub fn atmos(z: f32) -> Result<Atmosphere, MathError> {
    finite(&[z])?;
    // Gravity/derivative outputs (G, ELZ, DMDZ in the geopotential layers)
    // only feed ATMOS results DATCOM never reads, so they are not formed.
    let (tms, em, pressure);
    if z <= 295276.0 {
        let h = R0 * z / (R0 + z);
        // First layer whose upper bound reaches H; past the table, the last.
        let j = (1..10).find(|&i| HG[i] >= h).unwrap_or(9) - 1;
        let elh = (TM[j + 1] - TM[j]) / (HG[j + 1] - HG[j]);
        tms = TM[j] + elh * (h - HG[j]);
        em = WM0;
        pressure = if elh == 0.0 {
            PM[j] * (GMRS * (HG[j] - h) / tms).exp()
        } else {
            PM[j] * powr(TM[j] / tms, GMRS / elh)
        };
    } else {
        let i = (1..14).find(|&i| ZM[i] >= z).unwrap_or(13);
        let (j, k) = (i + 8, i - 1);
        let elz = (TM[j + 1] - TM[j]) / (ZM[k + 1] - ZM[k]);
        tms = TM[j] + elz * (z - ZM[k]);
        let dmdz = (WM[k + 1] - WM[k]) / (ZM[k + 1] - ZM[k]);
        em = WM[k] + dmdz * (z - ZM[k]);
        let zlz = z - tms / elz;
        let r = R0 / (R0 + zlz);
        pressure = PM[j]
            * (GMRS / elz
                * (r * r)
                * ((z - ZM[k]) * (R0 + zlz) / (R0 + z) / (R0 + ZM[k])
                    - (tms * (R0 + ZM[k]) / TM[j] / (R0 + z)).ln()))
            .exp();
    }
    let state = Atmosphere {
        sound_speed: 49.022164 * tms.sqrt(),
        pressure,
        density: GMRS * pressure / G0 / tms,
        temperature: em * tms / WM0,
    };
    finite(&[
        state.sound_speed,
        state.pressure,
        state.density,
        state.temperature,
    ])?;
    Ok(state)
}

/// Speed regime of one flight point, from MAIN00's STMACH/TSMACH tests.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Regime {
    Subsonic,
    Transonic,
    Supersonic,
}

/// One MAIN00 call: FLTCON values after the main program's per-point fill.
#[derive(Debug, Clone, PartialEq)]
pub struct FlightPoint {
    /// Zero-based Mach index (the original's OVERLY `I`).
    pub mach_index: usize,
    /// Zero-based altitude index (the original's `K`).
    pub altitude_index: usize,
    pub mach: f32,
    /// Reynolds number per unit length (RNNUB).
    pub reynolds: f32,
    /// VINF/ALT/PINF/TINF; values that were not input read -UNUSED.
    pub velocity: f32,
    pub altitude: f32,
    pub pressure: f32,
    pub temperature: f32,
    pub regime: Regime,
}

/// The main program's Mach/altitude sweep for a converted case (FLTCON
/// already processed by CHECK). Like the original, the fills accumulate in
/// the FLTCON arrays (e.g. MACH from VINF changes with each altitude).
pub fn flight_points(case: &Case) -> Result<Vec<FlightPoint>, MathError> {
    let fl = case.namelist("FLTCON").expect("FLTCON schema");
    let mut flc = fl.values.clone();
    let used = |v: f32| v != UNUSED;
    let nmach = ((flc[0] + 0.5) as i32).clamp(0, 20) as usize;
    let lp = (flc[159] + 0.5) as i32;
    let nalt = ((flc[158] + 0.5) as i32).clamp(0, 20) as usize;
    let (stmach, tsmach) = (flc[93], flc[94]);
    let im = used(flc[2]);
    let irn = used(flc[42]);
    let iatm = used(flc[96]) || (used(flc[116]) && used(flc[73]));
    let pairs: Vec<(usize, usize)> = match lp {
        1 => (0..nmach).map(|i| (i, i)).collect(),
        2 => (0..nalt)
            .flat_map(|k| (0..nmach).map(move |i| (i, k)))
            .collect(),
        3 => (0..nmach)
            .flat_map(|i| (0..nalt).map(move |k| (i, k)))
            .collect(),
        _ => Vec::new(),
    };
    let mut points = Vec::with_capacity(pairs.len());
    for (i, k) in pairs {
        let t = flc[k + 116];
        if im && iatm {
            flc[i + 136] = flc[i + 2] * 49.01685 * t.sqrt();
        }
        if !im {
            flc[i + 2] = flc[i + 136] / (49.01685 * t.sqrt());
        }
        if !irn {
            flc[i + 42] = 1.2527E6 * flc[k + 73] * flc[i + 2] * (t + 198.6) / (t * t);
        }
        for slot in [i + 136, k + 96, k + 73, k + 116] {
            if flc[slot] == UNUSED {
                flc[slot] = -UNUSED;
            }
        }
        let mach = flc[i + 2];
        finite(&[mach, flc[i + 42]])?;
        let regime = if mach >= tsmach {
            Regime::Supersonic
        } else if mach <= stmach {
            Regime::Subsonic
        } else {
            Regime::Transonic
        };
        points.push(FlightPoint {
            mach_index: i,
            altitude_index: k,
            mach,
            reynolds: flc[i + 42],
            velocity: flc[i + 136],
            altitude: flc[k + 96],
            pressure: flc[k + 73],
            temperature: flc[k + 116],
            regime,
        });
    }
    Ok(points)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn sea_level_is_the_1962_standard_atmosphere() {
        let a = atmos(0.0).unwrap();
        assert!((a.temperature - 518.67).abs() < 0.01);
        assert!((a.pressure - 2116.2).abs() < 0.5);
        assert!((a.sound_speed - 1116.4).abs() < 0.5);
    }
}
