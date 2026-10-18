//! M31O37: wing pitching moment (CMALPH, FWDXAC; CACALC is empty) and the
//! slope pass that finishes the subsonic wing, DATCOM 4.1.4.
//! Aerodynamic centre: high aspect ratio from XAC/A(30); otherwise FIG
//! 4.1.4.2-22A/B (FWDXAC for forward sweep), area-weighted over the inboard
//! and outboard panels of non-straight planforms. Above CLmax the
//! nonlinear 4.1.4.3 method uses FIG 4.1.4.3-11/12/13.

use super::{data::cmalph as t, data::fwdxac as x, tb, tl, WingCase, STRA};
use crate::{
    aero::Result,
    input::UNUSED,
    interp::{self, Extrapolation, Limits},
    tables::{PI, RAD},
};

/// FIG 4.1.4.2-22A (`factor` = tan(LE sweep)/beta below 1) or -22B
/// (beta/tan above): XAC/cr from (AR tan(LE), factor, taper).
fn fig22(atn: f32, factor: f32, taper: f32, b: bool) -> Result<f32> {
    let flags = [2, 0, 0, 2, 0, 0];
    if b {
        tl(
            &[&t::X122B, &t::X222B, &t::X322B],
            &t::Y22B,
            &[atn, factor, taper],
            &flags,
        )
    } else {
        tl(
            &[&t::X122A, &t::X222A, &t::X322A],
            &t::Y22A,
            &[atn, factor, taper],
            &flags,
        )
    }
}

/// FWDXAC: aerodynamic centre of forward-swept panels; the four tables
/// are chosen by Mach and |factor| (factor or its inverse as the axis).
pub(super) fn fwdxac(atnswp: f32, taper: f32, factor: f32, mach: f32) -> Result<f32> {
    let afact = factor.abs();
    let facti = if afact != 0. { 1. / afact } else { 1.0e10 };
    let (table, x2): (&[f32], f32) = match (mach < 1., afact <= 1.) {
        (true, true) => (&x::TYSUBL, afact),
        (true, false) => (&x::TYSUPR, facti),
        (false, false) => (&x::TYSUPL, facti),
        (false, true) => (&x::TYSUPR, afact),
    };
    tl(
        &[&x::TCURVE, &x::TX, &x::TGRAPH],
        table,
        &[atnswp.abs(), x2, taper],
        &[2, 0, 0, 2, 0, 0],
    )
}

impl WingCase {
    /// CMALPH: C(1..51), WING(41..60) = CM, WING(121) = CMalpha and B(47) =
    /// CM0. A zero sweep A(38) (straight) or A(62) (others) is replaced by
    /// a small value that, unlike CDRAG's, the original never restores.
    pub(super) fn cmalph(&mut self) -> Result<()> {
        let (sref, cbarr, n) = (self.sref, self.cbarr, self.alphas.len());
        let (a, b, c, ain) = (&mut self.a, &mut self.b, &mut self.c, &self.ain);
        c[1] = ain[61];
        c[2] = ain[67];
        a[170] = a[173] / a[10];
        c[48] = a[10] / cbarr;
        if !(c[1].abs() < 1.0e-10 || c[2].abs() < 1.0e-10) {
            c[1] = 0.50 * (c[1] + c[2]);
        }
        let calm = tb(&t::XCMOM, &t::YCMOM, b[1], 0, 0)?;
        let calm = a[3] * a[16] * calm / (sref * cbarr);
        c[5] = (a[7] * (a[43] * a[43]) / (a[7] + 2.0 * a[43])) * c[1] * calm;
        if ain[11].abs() >= 1.0e-20 {
            c[4] = tl(
                &[&t::X11412, &t::X21412, &t::X31412],
                &t::Y41412,
                &[a[7], a[40], a[27]],
                &[2, 2, 0, 2, 2, 0],
            )?;
            c[5] += c[4] * ain[11] * calm;
        }
        c[3] = calm;
        b[47] = c[5];
        c[9] = a[7] * a[38];
        if self.kind == STRA {
            if a[38] == 0. {
                a[38] = 0.0001;
            }
            c[10] = a[38] / b[2];
            c[11] = b[2] / a[38];
            c[6] = if a[7] > a[125] {
                let xac = ain[self.i + 71];
                let mut c6 = (a[30] - 0.25 * a[16]) / a[10];
                if xac != UNUSED {
                    c6 += a[16] * (xac - 0.25) / a[10];
                }
                c6
            } else if a[38] < 0.0 {
                fwdxac(c[9], a[27], c[10], b[1])?
            } else if c[10] >= 1.0 {
                fig22(c[9], c[11], a[27], true)?
            } else {
                fig22(c[9], c[10], a[27], false)?
            };
        } else {
            if a[62] == 0.0 {
                a[62] = 1.0e-15;
            }
            c[12] = a[5] * a[62];
            c[13] = a[62] / b[2];
            c[14] = 1.0 / c[13];
            c[15] = a[168] * a[86];
            c[16] = a[86] / b[2];
            c[17] = 1.0 / c[16];
            // Outboard panel: FWDXAC only when both panels sweep forward.
            let outboard_forward = if a[62] < 0.0 {
                c[18] = fwdxac(c[12], a[26], c[13], b[1])?;
                a[86] < 0.0
            } else {
                c[18] = fig22(
                    c[12],
                    if c[13] >= 1.0 { c[14] } else { c[13] },
                    a[26],
                    c[13] >= 1.0,
                )?;
                false
            };
            c[19] = if outboard_forward {
                fwdxac(c[15], a[169], c[16], b[1])?
            } else if c[16] >= 1.0 {
                fig22(c[15], c[17], a[169], true)?
            } else {
                fig22(c[15], c[16], a[169], false)?
            };
            c[20] = c[19] * a[166] / a[10] - a[164] * a[86] / a[10] + a[23] * a[62] / a[10];
            let anum = a[171] * a[1] * c[18] + a[172] * a[167] * c[20];
            let aden = a[171] * a[1] + a[172] * a[167];
            c[6] = anum / aden;
        }
        c[7] = (a[170] - c[6]) * (a[10] / cbarr);
        c[8] = c[7] * self.wing[101];
        self.wing[121] = c[8];
        let w = &mut self.wing;
        if a[7] > 6. / a[124] || a[34] < 0.0 {
            for k in 1..=n {
                w[k + 40] = w[k + 60] * c[7] + c[5];
            }
            return Ok(());
        }
        c[21] = a[7] * (1. + a[27]) * a[38] / 4.0;
        c[22] = (a[27] + c[21] + ((1. + c[21] * a[27]) / (1. + a[27]))) / 3.0;
        if !(a[7] >= 6. / a[124] || self.kind == STRA) {
            let ybsi = a[23] * (1. + 2. * a[26]) / (3. * (1. + a[26]));
            let xble =
                (ybsi * a[1] * a[62] + (a[23] * a[62] + (a[133] - a[21]) * a[86]) * a[2]) / a[3];
            c[22] = (xble + a[16] / 2.0) / a[10];
        }
        c[23] = tb(&t::X211B, &t::Y11B, a[27], 0, 0)?;
        c[24] = (c[23] + 1.0) * c[9];
        c[25] = tl(
            &[&t::X111C, &t::X211C],
            &t::Y11C,
            &[a[7], c[24]],
            &[2, 0, 2, 1],
        )?;
        c[26] = tb(&t::X211A, &t::Y11A, ain[17], 2, 2)?;
        c[27] = c[25] + c[26];
        c[28] = (b[43] / RAD).sin();
        c[29] = (b[43] / RAD).tan();
        c[30] = c[27] / c[28] - c[6] / c[29];
        let flags = [2, 0, 2, 0];
        let b13b2 = |x: f32, y: f32| tl(&[&t::X113B2, &t::X213B2], &t::Y13B2, &[x, y], &flags);
        let b12b2 = |x: f32, y: f32| tl(&[&t::X112B2, &t::X212B2], &t::Y12B2, &[x, y], &flags);
        for m in 1..=n {
            let alpha = b[m + 22];
            c[31] = (alpha / RAD).sin();
            c[32] = (alpha / RAD).cos();
            c[33] = c[31] / c[32];
            c[34] = a[7] * a[37];
            c[35] = (c[33] / c[29]).abs();
            let tanrat = c[33] > c[29];
            if m == 1 {
                c[36] = ((b[43] / RAD).tan() / 0.6).atan();
                let temp2 = c[29] / c[36].tan();
                c[47] = temp2;
                c[40] = tl(
                    &[&t::X112A, &t::X212A],
                    &t::Y12A,
                    &[c[9], a[27]],
                    &[1, 0, 1, 0],
                )?;
                c[37] = tl(
                    &[&t::X113A, &t::X213A],
                    &t::Y13A,
                    &[c[34], a[27]],
                    &[0, 0, 2, 0],
                )?;
                c[51] = b13b2(temp2, c[37])?;
                c[50] = b12b2(temp2, c[40])?;
                c[49] = c[6] * c[36].cos() + c[36].sin() * (c[30] + c[51] + c[50]);
            }
            if alpha / RAD > c[36] {
                c[41] = c[22] - c[49];
                c[42] = PI / 2. - c[36];
                c[43] = c[41] / c[42];
                c[44] = c[49] + c[43] * (alpha / RAD - c[36]);
                w[m + 40] = w[m + 60] * (a[170] - c[44]) * (a[10] / cbarr) + c[5];
            } else {
                if tanrat {
                    c[46] = c[29] / c[33];
                    c[38] = b13b2(c[46], c[37])?;
                    c[39] = b12b2(c[46], c[40])?;
                } else {
                    c[38] = tl(
                        &[&t::X113B1, &t::X213B1],
                        &t::Y13B1,
                        &[c[35], c[37]],
                        &flags,
                    )?;
                    c[39] = tl(
                        &[&t::X112B1, &t::X212B1],
                        &t::Y12B1,
                        &[c[35], c[40]],
                        &flags,
                    )?;
                }
                c[44] = c[6] * c[32] + (c[30] + c[39] + c[38]) * c[31];
                w[m + 40] = w[m + 60] * (a[173] / a[10] - c[44]) * c[48] + c[5];
            }
        }
        Ok(())
    }

    /// The rest of M31O37: CN/CA from CL and CD at the FLTCON angles,
    /// CLalpha/CMalpha per alpha as curve slopes (at alpha = 0 when EXPR
    /// wing data were given), and moments past a > 90% slope change marked
    /// unreliable (2*UNUSED) for nonlinear methods.
    pub(super) fn slopes(&mut self, kwing: bool) -> Result<()> {
        let n = self.alphas.len();
        let al = &self.alphas;
        let w = &mut self.wing;
        let hold = Limits {
            lower: Extrapolation::Hold,
            upper: Extrapolation::Hold,
        };
        let slope = |w: &[f32; 401], base: usize, at: f32| -> Result<f32> {
            Ok(interp::tbfunx_unordered(al, &w[base..base + n], at, hold)?.derivative)
        };
        let mut cla = w[101];
        let mut cma = w[121];
        for j in 1..=n {
            let ca = (al[j - 1] / RAD).cos();
            let sa = (al[j - 1] / RAD).sin();
            w[j + 60] = w[j + 20] * ca + w[j] * sa;
            w[j + 80] = w[j] * ca - w[j + 20] * sa;
            w[j + 100] = slope(w, 21, al[j - 1])?;
            w[j + 120] = slope(w, 41, al[j - 1])?;
        }
        if kwing {
            cla = slope(w, 21, 0.)?;
            cma = slope(w, 41, 0.)?;
        }
        if !((self.a[7] <= 6. / self.a[124] && self.kind == STRA) || kwing) {
            let mut flag = false;
            for j in 2..=n {
                let del = 100. * (w[j + 100] / cla - 1.0).abs();
                flag |= del > 90.0;
                if flag {
                    w[j + 40] = 2.0 * UNUSED;
                    w[j + 120] = 2.0 * UNUSED;
                }
            }
        }
        w[101] = cla;
        w[121] = cma;
        Ok(())
    }
}
