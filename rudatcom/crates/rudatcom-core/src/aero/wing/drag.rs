//! M03O03: wing drag (CDRAG), DATCOM 4.1.5. Zero-lift skin-friction drag
//! (FIG 4.1.5.1-26/27/28B) plus drag due to lift: straight tapered wings use
//! the span-efficiency method with twist terms (FIG 4.1.5.2-42/48/53),
//! cranked wings blend inboard/outboard leading-edge suction, and other
//! planforms use 0.95 CL tan(alpha).

use super::{
    data::{cdrag as t, fig53a as f},
    tb, tl, WingCase, CRAN, STRA,
};
use crate::{
    aero::{
        body::{fig26, X27I, X27M},
        Result,
    },
    math::powr,
    tables::{PI, RAD},
};

/// FIG53A: leading-edge suction parameter R from the polynomial fits in
/// log10(Reynolds number) for the bracketing Z = AR*lambda/cos(sweep)
/// curves, linearly blended and clipped at zero.
fn fig53a(rv: f32, z: f32) -> f32 {
    if rv == 0.0 {
        return 0.0;
    }
    let x = rv.log10();
    let poly = |m: usize| {
        f::A[m] * x.powi(5)
            + f::B[m] * x.powi(4)
            + f::C[m] * x.powi(3)
            + f::D[m] * x.powi(2)
            + f::E[m] * x
    };
    let mut r = poly(4);
    for m in 0..4 {
        if z >= f::ZZ[m] && z < f::ZZ[m + 1] {
            let ym = poly(m);
            r = if (z - f::ZZ[m]).abs() <= 0.001 {
                ym
            } else {
                ym + ((z - f::ZZ[m]) / (f::ZZ[m + 1] - f::ZZ[m])) * (poly(m + 1) - ym)
            };
            break;
        }
    }
    r.max(0.0)
}

/// Leading-edge suction R: FIG53A's fits up to a Reynolds number of
/// 1.3e5, the FIG 4.1.5.2-53B table at `z` above it.
fn suction(rn: f32, z: f32) -> Result<f32> {
    if rn > 1.30e5 {
        tb(&t::X53B, &t::Y53B, z, 0, 0)
    } else {
        Ok(fig53a(rn, z))
    }
}

impl WingCase {
    /// CDRAG: fills D(1..55), WING(1..20) = CD and B(46) = CD0. Zero sweeps
    /// are temporarily 1e-5 to avoid dividing by them; with a body the lift
    /// is scaled by FIG 4.3.1.2-10's K(W(B)) for the drag-due-to-lift terms.
    pub(super) fn cdrag(&mut self) -> Result<()> {
        let n = self.alphas.len();
        let (sref, fm) = (self.sref, self.mach);
        let (a, d, ain) = (&mut self.a, &mut self.d, &self.ain);
        d[2] = 12.0 * a[16] / self.roughness;
        d[3] = 1.0;
        d[9] = a[129] * ain[62] * a[15];
        d[8] = a[129] * ain[63] * a[17];
        for k in [38, 62, 86] {
            if a[k] == 0.0 {
                a[k] = 0.00001;
            }
        }
        let w = &mut self.wing;
        let saved = if self.body {
            let ratio = (ain[4] - ain[3]) / ain[4];
            let rkwb = tb(&t::X1OA, &t::Y1OA, ratio, 0, 0)?;
            let saved = (w[101], w[21..21 + n].to_vec());
            w[101] *= rkwb;
            for k in 0..n {
                w[k + 21] *= rkwb;
            }
            Some(saved)
        } else {
            None
        };
        // FIG 4.1.5.1-27 cutoff Reynolds number exponent.
        let cept = tb(&X27M, &X27I, fm, 0, 0)?;
        let cutoff = powr(d[2], 1.0482) * powr(10.0, cept);
        if ain[15] == f32::from(STRA) {
            d[17] = a[16] * a[129];
            d[14] = cutoff;
            if d[14] < d[17] {
                d[17] = d[14];
            }
            d[10] = fig26(d[17], fm);
            d[13] = tl(
                &[&t::X128B, &t::X228B],
                &t::Y28B,
                &[fm, a[178]],
                &[0, 2, -1, 2],
            )?;
            let capl = if ain[18] >= 0.30 { 1.20 } else { 2.0 };
            d[20] =
                d[10] * (1. + capl * ain[16] + 100. * ain[16].powi(4)) * d[13] * (2. * a[3] / sref);
            d[26] = a[129] * ain[62] * a[16];
            d[27] = d[26] / a[38].abs() * (1.0 - (fm * fm * (a[37] * a[37]))).sqrt();
            d[28] = a[7] * a[27] / a[37];
            d[29] = suction(d[27], d[28])?;
            let temp = (w[101] * sref / a[3]) * RAD / a[7];
            d[30] = 1.1 * temp / (d[29] * temp + (1. - d[29]) * PI);
            d[31] = a[7] * self.b[2];
            let angle = (a[44] / self.b[2]).atan() * RAD;
            let at = [angle, d[31], a[27]];
            let flags = [0, 2, 0, 2, 2, 0];
            d[33] = tl(&[&t::X142, &t::X242, &t::X342], &t::Y42, &at, &flags)?;
            d[32] = tl(&[&t::X148, &t::X248, &t::X348], &t::Y48, &at, &flags)?;
            let wtw = d[32] / self.b[2];
            let temp = PI * ain[11] / RAD;
            let temp2 = 2.0 * temp;
            for m in 1..=n {
                let clj = w[m + 20];
                d[34] = (clj * clj / (PI * a[7] * d[30])) * sref / a[3]
                    + temp2 * clj * d[33]
                    + temp2 * temp2 * wtw * a[3] / sref;
                d[m + 35] = d[34];
                d[35] = d[20] + d[34];
                w[m] = d[35];
            }
        } else {
            // Inboard (A(1)) and outboard (A(2)) panels separately.
            d[18] = a[15] * a[129];
            d[15] = cutoff;
            if d[15] < d[18] {
                d[18] = d[15];
            }
            d[11] = fig26(d[18], fm);
            d[23] = tl(
                &[&t::X128BD, &t::X228BD],
                &t::Y28BD,
                &[fm, a[190]],
                &[-1, 0, 0, 0],
            )?;
            let capl = if ain[18] < 0.30 { 2.00 } else { 1.20 };
            d[21] =
                d[11] * (1. + capl * ain[16] + 100. * ain[16].powi(4)) * d[23] * (2. * a[1] / sref);
            d[19] = a[17] * a[129];
            d[16] = d[15];
            if d[16] < d[19] {
                d[19] = d[16];
            }
            d[12] = fig26(d[19], fm);
            d[24] = tl(
                &[&t::X128B, &t::X228B],
                &t::Y28B,
                &[fm, a[184]],
                &[0, 2, 0, 2],
            )?;
            let capl = if ain[66] < 0.30 { 2.00 } else { 1.20 };
            d[22] =
                d[12] * (1. + capl * ain[65] + 100. * ain[65].powi(4)) * d[24] * (2. * a[2] / sref);
            d[20] = d[21] + d[22];
            if ain[15] == f32::from(CRAN) {
                d[7] = (d[9] / a[62].abs()) * (1. - (fm * a[61]) * (fm * a[61])).sqrt();
                d[5] = suction(d[7], 0.0)?;
                d[6] = (d[8] / a[86].abs()) * (1. - (fm * a[85]) * (fm * a[85])).sqrt();
                let tempo = a[6] * a[28] / a[85];
                d[4] = suction(d[6], tempo)?;
                d[1] = d[5] * a[23] / ain[3] + d[4] * (1. - (a[23] / ain[3]));
                let temp = (w[101] * sref / a[3]) * RAD / a[7];
                d[30] = temp / (d[1] * temp + (1. - d[1]) * PI);
                for m in 1..=n {
                    let temp = (w[m + 20].abs() / a[7]) * sref / a[3];
                    d[25] = tl(&[&t::X154, &t::X254], &t::Y54, &[temp, a[7]], &[0, 0, 0, 0])?;
                    d[34] = (w[m + 20] * w[m + 20] / (PI * d[30] * a[7])) * sref / a[3]
                        + d[25] * a[3] / sref;
                    d[m + 35] = d[34];
                    d[35] = d[20] + d[34];
                    w[m] = d[35];
                }
            } else {
                for m in 1..=n {
                    d[34] = 0.95 * w[m + 20] * (self.b[m + 22] / RAD).tan() * d[3];
                    d[m + 35] = d[34];
                    d[35] = d[20] + d[34];
                    w[m] = d[35];
                }
            }
        }
        self.b[46] = d[20];
        for k in [38, 62, 86] {
            if a[k] == 0.00001 {
                a[k] = 0.0;
            }
        }
        if let Some((w101, cl)) = saved {
            w[101] = w101;
            w[21..21 + n].copy_from_slice(&cl);
        }
        Ok(())
    }
}
