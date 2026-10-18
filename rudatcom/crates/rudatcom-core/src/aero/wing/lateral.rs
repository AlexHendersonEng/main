//! M29O35's wing defaults and M17O21's SUBLAT wing part: subsonic
//! sideslip derivatives (DATCOM 5.1.1/5.1.2). WING(141..160) and
//! WING(161..180) scale with CL and CL^2; WING(181..200) is the dihedral
//! effect from FIG 5.1.2.1-27..31 (TLIN4X for partial-span dihedral).
//! The wing-body (BO) and tail parts follow in later checkpoints.

use super::{data::sublat as t, tl, WingCase, STRA};
use crate::{
    aero::Result,
    input::UNUSED,
    tables::{PI, RAD},
};

impl WingCase {
    /// M29O35 without a body: dihedral span fraction STB(2), defaults for
    /// DHDADI/SSPNDD/DHDADO (kept in WINGIN for the rest of the case) and
    /// the span-averaged dihedral STB(122).
    pub(super) fn m29o35(&mut self) {
        let (w, s) = (&mut self.ain, &mut self.stb);
        s[2] = (w[4] - w[12]) / w[4];
        s[3] = 1.0;
        if w[13] == UNUSED {
            w[13] = 0.0;
        }
        if w[12] == UNUSED {
            w[12] = 0.0;
        }
        if w[14] == UNUSED {
            w[14] = w[13];
        }
        s[122] = (w[13] * (w[3] - w[12]) + w[14] * w[12]) / w[3];
    }

    /// SUBLAT for the wing at a subsonic point. Uses the CLMCH0 pre-pass's
    /// incompressible CL curve B(3..22) for the CL^2 terms.
    pub(super) fn sublat(&mut self) -> Result<()> {
        let (n, fm, sref, cbarr, blref) = (
            self.alphas.len(),
            self.mach,
            self.sref,
            self.cbarr,
            self.blref,
        );
        let (a, b, s, w, ain) = (&self.a, &self.b, &mut self.stb, &mut self.wing, &self.ain);
        let mut ya = [UNUSED; 8]; // YA27, 28A, 28B, 29, 30A, 30B, 31I, 31O
        let bdat = 2. * ain[4];
        let anum1 = 6.0 * a[44] * a[42];
        let den1 = PI * a[120] * (a[120] + 4. * a[43]);
        let bb = (1. - (fm * a[43]) * (fm * a[43])).sqrt();
        let anum2 = a[120] + 4.0 * a[43];
        let den2 = a[120] * bb + 4.0 * a[43];
        let apart = 1.0 / (4.0 * PI * a[120]);
        let bpart = a[44].abs() / (PI * a[120] * (a[120] + 4.0 * a[43]));
        let cpart = a[43] - 0.5 * a[120] - a[120] * a[120] / (8. * a[43]);
        let epart = (a[120] + 4. * a[43]) / (a[120] * bb + 4. * a[43]);
        let anum3 = (a[120] * bb) * (a[120] * bb) + 4. * a[120] * bb * a[43] - 8.0 * a[43] * a[43];
        let den3 = a[120] * a[120] + 4.0 * a[120] * a[43] - 8.0 * a[43] * a[43];
        for j in 1..=n {
            s[j + 15] = (1. / RAD) * (b[j + 2] * b[j + 2]) * (anum1 / den1) - 0.0001 * s[122].abs();
            if b[j + 2] == 0.0 {
                s[j + 35] = 0.0;
                w[j + 140] = 0.0;
            } else {
                s[j + 35] = (anum2 / den2) * (s[j + 15] / b[j + 2]);
                w[j + 140] = s[j + 35] * w[j + 20];
            }
            let dpart = if w[j + 60] == 0.0 {
                0.0
            } else {
                6.0 * (w[j + 40] / w[j + 60] / cbarr) * (a[42].abs() / a[120])
            };
            s[j + 75] = (apart - bpart * (cpart + dpart)) / RAD;
            w[j + 160] = epart * (anum3 / den3) * s[j + 75] * (w[j + 20] * w[j + 20]);
            w[j + 160] = w[j + 160] * bdat * sref / (blref * a[3]);
        }
        let fig27 = |x1: f32, x2: f32, x3: f32| {
            tl(
                &[&t::X127, &t::X227, &t::X327],
                &t::Y27,
                &[x1, x2, x3],
                &[0, 1, 0, 2, 2, 0],
            )
        };
        let fig28a =
            |x1: f32, x2: f32| tl(&[&t::X128A, &t::X228A], &t::Y28A, &[x1, x2], &[0, 2, 1, 2]);
        let fig28b =
            |x1: f32, x2: f32| tl(&[&t::X128B, &t::X228B], &t::Y28B, &[x1, x2], &[0, 0, 0, 0]);
        if self.kind != STRA {
            // Inboard (A(163), A(25)) and outboard (A(168), A(169)) panels.
            // The original's YA27 = ABS(YA27) lines here act on the unused
            // YA27, so the table values keep their sign.
            s[135] = -2.0 / (3.0 * RAD * a[163]);
            s[132] = fig27(a[163], a[70].abs(), a[25])?;
            s[134] = fig28a(a[163] / a[73], a[73] * fm)?;
            s[133] = fig28b(a[25], a[163])?;
            s[131] = -2.0 / (3. * RAD * a[168]);
            s[128] = fig27(a[168], a[94].abs(), a[169])?;
            s[130] = fig28a(a[168] / a[97], a[97] * fm)?;
            s[129] = fig28b(a[169], a[168])?;
            s[58] = a[171] * (a[1] / sref) + a[172] * (a[167] / sref);
            s[127] = if a[163] >= 1.0 {
                a[171] * (a[1] / sref) * (s[132] * s[134] + s[133]) * (ain[3] - ain[2]) / ain[4]
            } else {
                a[171] * (a[1] / sref) * s[135] * (ain[3] - ain[2]) / ain[4]
            };
            s[126] = if a[168] >= 1.0 {
                a[172] * (a[167] / sref) * (s[128] * s[130] + s[129])
            } else {
                a[172] * (a[167] / sref) * s[131]
            };
            for j in 1..=n {
                w[j + 180] = (w[j + 20] / s[58]) * (s[127] + s[126]) * bdat / blref;
            }
        } else if a[120] < 1.0 {
            for j in 1..=n {
                w[j + 180] = -2. / (RAD * 3. * a[120]) * w[j + 20]
                    - s[122] * a[120] / (6. * (RAD * RAD)) * (a[4] / sref);
                w[j + 180] = w[j + 180] * bdat / blref;
            }
        } else {
            let xa2 = fm * a[73];
            let x1arg = a[70].abs();
            let xa1 = a[120] / a[73];
            ya[0] = fig27(a[120], x1arg, a[118])?;
            if a[70] < 0.0 {
                ya[0] = ya[0].abs();
            }
            ya[1] = fig28a(xa1, xa2)?;
            ya[2] = fig28b(a[118], a[120])?;
            ya[5] = tl(
                &[&t::X130B, &t::X230B],
                &t::Y30B,
                &[a[118], a[120]],
                &[0, 2, 0, 2],
            )?;
            for j in 1..=n {
                s[j + 95] =
                    (ya[0] * ya[1] + ya[2]) * w[j + 20] * sref / a[4] + ain[11] * a[68] * ya[5];
            }
            if ain[12] == UNUSED || ain[12] == 0.0 {
                ya[4] = tl(
                    &[&t::X130A, &t::X230A],
                    &t::Y30A,
                    &[xa1, xa2],
                    &[2, 2, 2, 2],
                )?;
                ya[3] = tl(
                    &[&t::X129, &t::X229, &t::X329],
                    &t::Y29,
                    &[x1arg, a[120], a[118]],
                    &[0, 0, 0, 2, 2, 0],
                )?;
                for j in 1..=n {
                    w[j + 180] = (s[j + 95] + ain[13] * ya[3] * ya[4]) * a[4] / sref;
                    w[j + 180] = w[j + 180] * bdat / blref;
                }
            } else {
                // Partial-span dihedral: FIG 5.1.2.1-31 at the inboard
                // (STB(2)) and full (STB(3)) span fractions.
                s[7] = a[131] / (2.0 * PI) * RAD;
                let x1arg = (a[68] / b[2]).atan() * RAD;
                let x3arg = b[2] * a[120] / s[7];
                let akovb = s[7] / b[2];
                let fig31 = |eta: f32| {
                    tl(
                        &[&t::X131, &t::X231, &t::X331, &t::X431],
                        &t::Y31,
                        &[x1arg, eta, x3arg, a[118]],
                        &[0, 0, 1, 0, 2, 2, 1, 0],
                    )
                };
                ya[6] = fig31(s[2])?;
                ya[7] = fig31(s[3])?;
                for j in 1..=n {
                    w[j + 180] = (s[j + 95]
                        + (ya[7] - ya[6]) * akovb * ain[14] / RAD
                        + ya[6] / RAD * akovb * ain[13])
                        * a[4]
                        / sref
                        * bdat
                        / blref;
                }
            }
        }
        for (slot, v) in [68, 71, 70, 67, 66, 69, 64, 65].into_iter().zip(ya) {
            s[slot] = v;
        }
        Ok(())
    }
}
