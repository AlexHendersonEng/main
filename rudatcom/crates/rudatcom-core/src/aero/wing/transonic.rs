//! Transonic wing-alone path (checkpoint 7f): MAIN03's once-per-case DOL2
//! pre-pass (SETUP2 states that rerun MAIN01 at Mach 0.6 and MAIN04 at
//! Mach 1.4), then per point M18O22's geometry restore, M24O30 (TRANWB ->
//! TRSONI with TRANWG and CLMXB1), M25O31 (TRANCM), M35O43 (WBTRA's
//! blanking, SECLEV's WINGCL and CNCA).
//!
//! TRA(1..108) (COMMON /SBETA/) holds the transonic intermediates, and
//! SECOND(1..23) (COMMON /LEVEL2/) the pre-pass's roll-damping ratios that
//! WINGCL reads. TRANF/TRANAC are the original's cubic fairings between
//! the subsonic and supersonic estimates.

use super::{
    data::{clmxb1 as m, suplng as s, trancm as cm, trsoni as d, wingcl as w},
    lim,
    supersonic::ix,
    tb, tl, WingCase, STRA,
};
use crate::{
    aero::{
        body::{fig26, X27I, X27M},
        Result,
    },
    flight::FlightPoint,
    input::{Namelist, UNUSED},
    interp::{self, Limits},
    math::{self, powr},
    tables::{PI, RAD},
};

/// A Fortran LOGICAL stored in a REAL slot (.TRUE. is the integer 1).
fn logical(value: bool) -> f32 {
    if value {
        f32::from_bits(1)
    } else {
        0.0
    }
}

/// The words after DEP55A/DEP55B in the original's static data: Q455A or
/// Q455B's Hollerith labels ("4.1.", "5.2-", "55A "/"55B ") and a zero
/// pad. WINGCL's 7x6x4 INTERX reads 168 values from the 164-value tables,
/// so these bit patterns are reproduced for taper > 0.5 at high AR.
fn dep55(table: &[f32; 164], label: u32) -> [f32; 168] {
    let mut dep = [0.0; 168];
    dep[..164].copy_from_slice(table);
    dep[164] = f32::from_bits(0x2E31_2E34);
    dep[165] = f32::from_bits(0x2D32_2E35);
    dep[166] = f32::from_bits(label);
    dep
}

/// TRANF (`dely` None) and TRANAC (`dely` Some): a cubic through the two
/// points bracketing `xval`, with end slopes `dyl`/`dyr` and averaged
/// chord slopes inside. TRANAC with 8 points shifts Y(5) by DELY while
/// fitting and shifts it back, which can leave Y(5) rounded differently;
/// hence `y` is mutable. TRANF clamps the result to be non-negative.
fn fair(x: &[f32], y: &mut [f32], dyl: f32, dyr: f32, dely: Option<f32>, xval: f32) -> f32 {
    let npt = x.len();
    // 1-based J: the first knot past XVAL (from 2), then step back one.
    let j = (2..npt).find(|&j| xval < x[j - 1]).unwrap_or(npt) - 1;
    let k = j + 1;
    let mut yp = [0.0f32; 2];
    for (n, slope) in yp.iter_mut().enumerate() {
        let mp = j + n;
        if mp == 1 {
            *slope = dyl;
        }
        if mp != 1 && mp != npt {
            let (l, c, r) = (mp - 2, mp - 1, mp);
            let sl = (y[l] - y[c]) / (x[l] - x[c]);
            let sr = (y[c] - y[r]) / (x[c] - x[r]);
            let angav = (sl.atan() + sr.atan()) / 2.0;
            *slope = angav.sin() / angav.cos();
            match dely {
                None => {
                    if y[c] <= 0.0 {
                        *slope = 0.0;
                    }
                    if npt > 10 && (sl.abs() <= UNUSED || sr.abs() <= UNUSED) {
                        *slope = 0.0;
                    }
                }
                Some(dely) => {
                    if npt == 8 && mp == 5 && dely < -UNUSED {
                        *slope = 0.0;
                    }
                }
            }
            if sr != 0.0 && sl / sr < 0.0 {
                *slope = 0.0;
            }
        }
        if mp == npt {
            *slope = dyr;
        }
    }
    // TRANAC's temporary DELY shift of the fifth (1-based) ordinate.
    let shift = dely.filter(|_| npt == 8);
    if let Some(dely) = shift {
        for idx in [j, k].into_iter().filter(|&i| i == 5) {
            y[idx - 1] += dely;
        }
    }
    let (xj, xk, yj, yk) = (x[j - 1], x[k - 1], y[j - 1], y[k - 1]);
    let x1s = xj * xj;
    let x2s = xk * xk;
    let x12f = xj - xk;
    let y12f = yj - yk;
    let x12s = x1s - x2s;
    let x12c = x1s * xj - x2s * xk;
    let y12p = yp[0] - yp[1];
    let red = 2.0 * xk * x12f - x12s;
    let grn = 3.0 * x2s * x12f - x12c;
    let yel = yp[1] * x12f - y12f;
    let e = 3.0 * x12s * red - 2.0 * x12f * grn;
    let a = (y12p * red - 2.0 * x12f * yel) / e;
    let b = (3.0 * x12s * yel - y12p * grn) / e;
    let c = (y12f - a * x12c - b * x12s) / x12f;
    let dd = yk - a * x2s * xk - b * x2s - c * xk;
    if let Some(dely) = shift {
        for idx in [j, k].into_iter().filter(|&i| i == 5) {
            y[idx - 1] -= dely;
        }
    }
    let yval = ((a * xval + b) * xval + c) * xval + dd;
    if dely.is_none() && yval < 0.0 {
        0.0
    } else {
        yval
    }
}

/// TRANF on read-only ordinates.
fn tranf(x: &[f32], y: &[f32], dyl: f32, dyr: f32, xval: f32) -> f32 {
    let mut y = y.to_vec();
    fair(x, &mut y, dyl, dyr, None, xval)
}

impl WingCase {
    /// MAIN03 for a wing-alone transonic point. DOL2 (wing straight
    /// tapered) enables the pre-pass, TRANCM and WINGCL; DONE in SECOND(15)
    /// limits the pre-pass to the first such point of the case.
    pub fn transonic(
        &mut self,
        point: &FlightPoint,
        experimental: Option<&Namelist>,
    ) -> Result<super::WingAero> {
        self.main00(point);
        let dol2 = self.kind == STRA;
        self.second[16] = logical(dol2);
        if self.second[15].to_bits() == 0 && dol2 {
            self.prepass()?;
        }
        // NF = 1: M51O63, EXSUBT, M18O22 (geometry only, as TRANSN).
        self.initz1();
        self.exsubt(experimental);
        self.m18o22_restore();
        self.trsoni(false)?;
        self.exsubt(experimental);
        if dol2 {
            self.trancm()?;
        }
        self.exsubt(experimental);
        // M35O43 -> WBTRA: only the first alpha survives in WING's nine
        // 20-slot blocks (CLB at 181.. is not blanked) unless EXPR gives it.
        self.exsubt(experimental);
        for block in 0..9 {
            for j in 2..=self.alphas.len() {
                self.wing[20 * block + j] = -UNUSED;
            }
        }
        self.exsubt(experimental);
        // SECLEV.
        if dol2 {
            self.exsubt(experimental);
            self.wingcl()?;
        }
        self.cnca()?;
        let result = self.result();
        // MAIN03 clears DOL2 after printing.
        self.second[16] = 0.0;
        result
    }

    /// The DOL2 pre-pass (NF < 0, so EXSUBT does nothing): TRSONI up to
    /// the force-break Mach, then SETUP2's states for a wing alone. States
    /// whose FLAG is false (body or tail cases) only set B/WINGIN/SECOND;
    /// MAIN01 runs at Mach 0.6 and MAIN04 at 1.4. B(1)/B(2) and WINGIN(I+20)
    /// keep SETUP2's last values afterwards, as in the original.
    fn prepass(&mut self) -> Result<()> {
        self.trsoni(true)?;
        let clamo = self.ain[69];
        let i = self.i;
        // State 1: save the Mach and force-break Machs (MFBH = TRAH(6) is
        // UNUSED without a horizontal tail).
        self.second[17] = self.mach;
        self.second[18] = self.tra[6];
        self.second[19] = UNUSED;
        self.set_mach(0.6, 0.8);
        self.ain[i + 20] = clamo / self.b[2];
        self.ain[i + 40] = self.ain[68];
        // The substitute-Mach runs may produce NaN CL (e.g. alpha 0 beyond a
        // negative alpha(CLmax)) that the original computes and discards.
        math::tolerate_non_finite(|| self.main01(None))?;
        // State 2 (WBT(67) and HT's CLBCLC are UNUSED without them).
        self.second[11] = UNUSED;
        self.second[1] = self.clbclc();
        self.second[3] = UNUSED;
        self.set_mach(0.7, 0.714_142_84);
        self.ain[i + 20] = clamo / self.b[2];
        // States 3 and 4: the wing's and tail's force-break Machs.
        for slot in [18, 19] {
            self.second[if slot == 18 { 12 } else { 5 }] = UNUSED;
            if self.second[slot] > 0.95 {
                self.second[slot] = 0.95;
            }
            let mfb = self.second[slot];
            self.set_mach(mfb, (1.0 - mfb * mfb).sqrt());
            self.ain[i + 20] = clamo / self.b[2];
        }
        // State 5: MAIN04 at Mach 1.4 with B left from state 4.
        self.second[7] = UNUSED;
        self.mach = 1.4;
        math::tolerate_non_finite(|| self.main04(None))?;
        // State 6: body and tail entries are UNUSED for a wing alone.
        self.second[2] = self.clbclc();
        for k in [4, 6, 8, 9, 10, 14] {
            self.second[k] = UNUSED;
        }
        // State 7: restore the Mach and mark the pre-pass DONE.
        self.second[13] = UNUSED;
        self.mach = self.second[17];
        self.second[15] = logical(true);
        for k in 17..=23 {
            self.second[k] = UNUSED;
        }
        Ok(())
    }

    /// SETUP2's FLC(I+2), B(1) and B(2) assignment.
    fn set_mach(&mut self, mach: f32, beta: f32) {
        self.mach = mach;
        self.b[1] = mach;
        self.b[2] = beta;
    }

    /// CLBCLC on WING: CLB/CL at the first alpha where both are set.
    fn clbclc(&self) -> f32 {
        (1..=self.alphas.len())
            .find(|&j| self.wing[j + 20].abs() > UNUSED && self.wing[j + 180].abs() != UNUSED)
            .map_or(UNUSED, |j| self.wing[j + 180] / self.wing[j + 20])
    }

    /// TRSONI's wing part: lift-curve slope faired between the subsonic
    /// force-break estimates and TRANWG's Mach 1.4 value, low-AR CLmax and
    /// alpha(CLmax), and CD0 with the faired wave drag. The pre-pass stops
    /// after the force-break Mach MFB (TRA(6)).
    fn trsoni(&mut self, prepass: bool) -> Result<()> {
        let mach = self.mach;
        self.tra[4] = mach;
        if self.kind != STRA {
            return Ok(());
        }
        let (cla14, dcla14) = self.tranwg()?;
        self.tra[1] = cla14;
        self.tra[82] = dcla14;
        let k = self.ain[69] * RAD / (2.0 * PI);
        self.tra[3] = k;
        let (toc, ar, tapr, cosle) = (self.ain[16], self.a[7], self.a[27], self.a[61]);
        let (srstar, sr, tanc2) = (self.a[3], self.sref, self.a[74]);
        // FIG 4.1.3.2-53A/B: force-break Mach, unswept then swept.
        let mfb0 = ix(
            &d::T43A,
            8,
            &[8, 7],
            &d::D43A,
            &[toc * 100.0, ar],
            [2, 2, 0, 0, 2, -1, 0, 0],
        )?;
        self.tra[5] = mfb0;
        let mfb = ix(
            &d::T43B,
            8,
            &[8, 5],
            &d::D43B,
            &[self.a[71], mfb0],
            [0, 2, 0, 0, 2, 0, 0, 0],
        )?;
        self.tra[6] = mfb;
        if prepass {
            return Ok(());
        }
        // FIG 4.1.3.2-54A/B: slope corrections above the force break.
        let var = [toc * 100.0, ar];
        let aoc = ix(
            &d::T44B,
            7,
            &[7, 6],
            &d::D44B,
            &var,
            [0, 2, 0, 0, 1, 2, 0, 0],
        )?;
        self.tra[7] = aoc;
        let cfbct = ix(
            &d::T44A,
            7,
            &[7, 6],
            &d::D44A,
            &var,
            [2, 1, 0, 0, 2, 2, 0, 0],
        )?;
        self.tra[8] = cfbct;
        let betafb = if mfb > 0.98 {
            0.0
        } else {
            (1.0 - mfb * mfb).sqrt()
        };
        self.tra[9] = betafb;
        // Helmbold-type slope at beta = BB: ARG1 / (2 + sqrt(ARG2*ARG3 + 4)).
        let arg1 = 2.0 * PI * ar / RAD;
        let q = ar / k;
        let arg2 = q * q;
        let arg4 = 2.0 + (arg2 * (betafb * betafb + tanc2 * tanc2) + 4.0).sqrt();
        let clafbt = arg1 / arg4;
        self.tra[10] = clafbt;
        let q = 0.8 * ar / k;
        let arg2_6 = q * q;
        let arg3 = if arg2_6 > 0.0 {
            let t = tanc2 / 0.8;
            1.0 + t * t
        } else {
            0.0
        };
        let arg4 = 2.0 + (arg2_6 * arg3 + 4.0).sqrt();
        self.tra[70] = arg1 * srstar / (arg4 * sr);
        // Subsonic end of the fairing: Mach XM <= 0.75, MFB - 0.1, MACH.
        let mut xm = 0.75f32;
        if xm > mfb - 0.1 {
            xm = mfb - 0.1;
        }
        if xm > mach {
            xm = mach;
        }
        let bb = (1.0 - xm * xm).sqrt();
        let arg3 = bb * bb + tanc2 * tanc2;
        let root = (arg2 * arg3 + 4.0).sqrt();
        let arg4 = 2.0 + root;
        let claw7 = arg1 * srstar / (arg4 * sr);
        let dcla7 = xm * arg1 * arg2 * srstar / sr;
        let dcla7 = dcla7 / (arg4 * arg4 * root);
        let clafb = clafbt * cfbct * srstar / sr;
        self.tra[12] = clafb;
        let claa = (1.0 - aoc) * clafb;
        self.tra[13] = claa;
        // FIG 4.1.3.2-54C (B/C).
        let boc = ix(
            &d::T44C,
            6,
            &[6],
            &d::D44C,
            &[toc * 100.0],
            [2, 0, 0, 0, 1, 0, 0, 0],
        )?;
        self.tra[14] = boc;
        let clab = (1.0 - boc) * clafb;
        self.tra[15] = clab;
        let mt = [xm, mfb, mfb + 0.07, mfb + 0.14, 1.4];
        let clamt = [claw7, clafb, claa, clab, cla14];
        self.tra[16..21].copy_from_slice(&mt);
        self.tra[21..26].copy_from_slice(&clamt);
        self.wing[101] = tranf(&mt, &clamt, dcla7, dcla14, mach);
        // Aspect-ratio classification (FIG 4.1.3.4-24B C1 and C2).
        let c1 = tb(&d::X24B, &d::Y24B, tapr, 0, 0)?;
        self.tra[27] = c1;
        let aratio = self.a[128] / ((c1 + 1.0) * cosle);
        self.tra[28] = aratio;
        let c2 = ix(&d::TR, 11, &[11], &d::DR, &[tapr], [0; 8])?;
        if ar < aratio {
            self.low_aspect_ratio(c1, c2, xm)?;
        }
        // CD0: skin friction at the cutoff Reynolds number (FIG 4.1.5.1-27)
        // plus wave drag (FIG 4.1.5.1-29) faired over the drag-rise Machs.
        let cbar = self.a[16];
        let arg = 12.0 * cbar / self.rough();
        let cept = tb(&X27M, &X27I, 0.6, 0, 0)?;
        let rlcoff = powr(arg, 1.0482) * powr(10.0, cept);
        self.tra[39] = rlcoff;
        let rnn = (cbar * self.a[129]).min(rlcoff);
        self.tra[40] = rnn;
        let cf = fig26(rnn, 0.6);
        self.tra[42] = cf;
        let rl = if self.ain[18] < 0.3 { 2.0 } else { 1.2 };
        self.tra[41] = rl;
        let cdf = cf * (1.0 + rl * toc) * 2.0 * srstar / sr;
        self.tra[68] = cdf;
        let cosc4 = self.a[67];
        let arg = cosc4.sqrt();
        let xmtd = [
            0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.925, 0.95, 0.975, 1.0, 1.05, 1.1, 1.2, 1.4,
        ]
        .map(|m: f32| m / arg);
        let arg2 = powr(toc, 0.3333);
        let var2 = ar * arg2;
        let arg3 = powr(cosc4, 2.5);
        for (n, &x) in xmtd.iter().enumerate() {
            let p = x * arg;
            let var1 = (p * p - 1.0).abs().sqrt() / arg2;
            // The supersonic (right-side) chart value is never stored, so
            // CDW2 keeps INITZ1's UNUSED there.
            if p <= 1.0 {
                let cdw1 = ix(
                    &d::T429L,
                    8,
                    &[8, 7],
                    &d::D429L,
                    &[var1, var2],
                    [0, 0, 0, 0, -1, 2, 0, 0],
                )?;
                self.tra[43 + n] = cdw1 * powr(toc, 1.666) * arg3 * srstar / sr;
            }
        }
        let cdw = tranf(&xmtd, &self.tra[43..58], 0.0, 0.0, mach);
        self.tra[67] = cdw;
        self.wing[1] = cdw + cdf;
        Ok(())
    }

    /// TRSONI's low-aspect-ratio branch: CLmax and alpha(CLmax) at Mach
    /// 0.6 (FIG 4.1.3.4-25A/B) adjusted to the Mach (FIG 4.1.3.4-26A-C).
    fn low_aspect_ratio(&mut self, c1: f32, c2: f32, xm: f32) -> Result<()> {
        let (ar, tapr, cosle, tanle) = (self.a[7], self.a[27], self.a[61], self.a[62]);
        let bu4 = (c1 + 1.0) * ar * cosle / 0.8;
        self.tra[29] = bu4;
        let arg = (c2 + 1.0) * ar * tanle;
        self.a[160] = arg;
        let clmax6 = self.clmxb1(bu4)?;
        self.tra[30] = clmax6;
        let aclbas = tb(&d::T418A, &d::D418A, bu4, 0, 0)?;
        self.tra[31] = aclbas;
        let dacma6 = if arg > 4.5 {
            tl(
                &[&d::T18B1, &d::D18B1],
                &d::C18B1,
                &[xm, arg],
                &[0, 0, 2, 1],
            )?
        } else {
            let arg1 = ar * cosle * (1.0 + 4.0 * (tapr * tapr));
            tl(
                &[&d::T18B2, &d::D18B2],
                &d::C18B2,
                &[arg1, arg],
                &[0, 2, 2, 0],
            )?
        };
        self.tra[32] = dacma6;
        let alclm6 = aclbas + dacma6;
        self.tra[36] = alclm6;
        let c3 = ix(&d::T419B, 13, &[13], &d::D419B, &[arg], [0; 8])?;
        self.tra[33] = c3;
        let var1 = bu4 * 0.8;
        let mach = self.mach;
        let dalcm = ix(
            &d::T419C,
            13,
            &[10, 13],
            &d::D419C,
            &[var1, mach],
            [2, 0, 0, 0, 2, 2, 0, 0],
        )?;
        self.tra[34] = dalcm;
        let dclmax = ix(
            &d::T419A,
            11,
            &[7, 11],
            &d::D419A,
            &[c3 * var1, mach],
            [2, 0, 0, 0, 2, 2, 0, 0],
        )?;
        self.tra[35] = dclmax;
        self.tra[37] = alclm6 + dalcm;
        self.tra[38] = clmax6 + dclmax * (self.a[3] / self.sref);
        Ok(())
    }

    /// TRANWG: supersonic normal-force slope at Mach 1.3/1.4/1.5 (as in
    /// SUPLNG, but without capping CNA/CNA_theory at 1), returning the Mach
    /// 1.4 value and its Mach derivative from a fit in beta. Zero leading-
    /// edge tangents A(62)/A(86) are bumped to 1e-5 for good.
    fn tranwg(&mut self) -> Result<(f32, f32)> {
        for k in [86, 62] {
            if self.a[k] == 0.0 {
                self.a[k] = 0.00001;
            }
        }
        let (ar, tanle, cosle, deltay) = (self.a[7], self.a[62], self.a[61], self.ain[17]);
        let rectangular = self.a[25] == 1.0 && self.a[58] == 0.0;
        let xm = [1.3f32, 1.4, 1.5];
        let mut cn = [0.0f32; 3];
        let mut bb = [0.0f32; 3];
        for i in 0..3 {
            let beta = (xm[i] * xm[i] - 1.0).sqrt();
            bb[i] = beta;
            let bovert = beta / tanle;
            let suple = bovert > 1.0;
            let flags = [2, 2, 0, 0, 0, 2, 0, 0];
            // FIG 4.1.3.2-60A/B: (CNA/CNA)theory for the edge type.
            let cncnt = if suple {
                let deltdt = (deltay / (5.85 * cosle)).atan() * RAD;
                ix(
                    &s::B1350,
                    11,
                    &[11, 8],
                    &s::DB50,
                    &[1.0 / bovert, deltdt],
                    flags,
                )?
            } else {
                ix(
                    &s::A1350,
                    9,
                    &[9, 8],
                    &s::DA50,
                    &[bovert, deltay / cosle],
                    flags,
                )?
            };
            let cnthry = if rectangular {
                if ar * beta > 1.0 {
                    let bcna = 4.0 - 2.0 * (1.0 / (ar * beta));
                    bcna / beta
                } else {
                    let cnaa = ix(&s::G13246, 9, &[9], &s::DG3246, &[ar * beta], [0; 8])?;
                    cnaa * ar
                }
            } else {
                let bcna = ix(
                    &s::T13246,
                    23,
                    &[23, 8, 6],
                    &s::D13246,
                    &[bovert, ar * tanle, self.a[27]],
                    [0, 2, 0, 0, 0, 2, 0, 0],
                )?;
                if suple {
                    bcna / beta
                } else {
                    bcna / tanle
                }
            };
            cn[i] = cnthry * cncnt * self.a[3] / (self.sref * RAD);
        }
        // CN*beta^2 = AA*beta^2 + B*beta + C, differentiated at Mach 1.4.
        let f1 = (cn[1] * bb[1].powi(2) - cn[0] * bb[0].powi(2)) / (bb[1] - bb[0]);
        let f2 = (cn[2] * bb[2].powi(2) - cn[0] * bb[0].powi(2)) / (bb[2] - bb[0]);
        let aa = (f2 - f1) / (bb[2] - bb[1]);
        let b = f1 - aa * (bb[1] + bb[0]);
        let c = cn[1] * bb[1].powi(2) - aa * bb[1].powi(2) - b * bb[1];
        let dcna = -b * xm[1] / bb[1].powi(3) - 2.0 * c * xm[1] / bb[1].powi(4);
        Ok((cn[1], dcna))
    }

    /// CLMXB1: low-AR CLmax at Mach 0.6 (FIG 4.1.3.4-16/17), A(160) being
    /// TRSONI's (C2 + 1) AR tan(LE sweep).
    fn clmxb1(&self, bu4: f32) -> Result<f32> {
        let dclmax = tl(
            &[&m::AMN, &m::C2A],
            &m::DE,
            &[0.6, self.a[160]],
            &[-1, 2, 0, 2],
        )?;
        let base = if self.ain[18] > 0.35 {
            &m::CBASE[114..]
        } else {
            &m::CBASE[..114]
        };
        let clbase = tl(
            &[&m::DYAG, &m::C1ABC],
            base,
            &[self.ain[17], bu4],
            &[0, 0, -1, 0],
        )?;
        Ok((clbase + dclmax) * self.a[3] / self.sref)
    }

    /// TRANCM's wing part: aerodynamic centre at the transonic Machs
    /// (FIG 4.1.4.2-25), the subsonic/supersonic ends (FIG 4.1.4.2-22) and,
    /// for thick sections, the FIG 4.1.4.2-28 shift faired through the
    /// force-break region; then CMA = (XCG - XW - XAC) CLA / CBARR.
    fn trancm(&mut self) -> Result<()> {
        let (ar, tanle, tapr, tovc) = (self.a[7], self.a[62], self.a[27], self.ain[16]);
        let var1 = ar * powr(tovc, 0.3333);
        let var2 = ar * tanle;
        let t = &cm::T425AD;
        let axes: [&[f32]; 4] = [&t[9..18], &t[0..7], &t[18..22], &t[27..30]];
        for (i, vbar) in (2..=5).zip([-2.0f32, -1.0, 0.0, 1.0]) {
            self.tra[82 + i] = (1.0 + vbar * powr(tovc, 0.6666)).sqrt();
            self.tra[88 + i] = tl(
                &axes,
                &cm::D425AD,
                &[var2, var1, vbar, tapr],
                &[0, 0, 0, 0, 2, 1, 0, 1],
            )?;
        }
        // Wing alone: the moment reference is XCG - XW.
        self.a[173] = self.syna.0 - self.syna.1;
        self.tra[83] = 0.6;
        self.tra[88] = 1.4;
        let flags = [0, 2, 0, 0, 0, 2, 0, 0];
        let xm = [0.5f32, 0.7, 0.6, 1.3, 1.5, 1.4];
        let mut dxac1 = 0.0;
        for (i, &xm) in xm[..3].iter().enumerate() {
            let arg1 = tanle / (1.0 - xm * xm).sqrt();
            let (table, at) = if arg1 > 1.0 {
                (&cm::SUBAF2, 1.0 / arg1)
            } else {
                (&cm::SUBAF1, arg1)
            };
            self.tra[89] = ix(&cm::T422AF, 6, &[6, 6, 6], table, &[at, var2, tapr], flags)?;
            match i {
                0 => dxac1 = self.tra[89],
                1 => dxac1 = (self.tra[89] - dxac1) / 0.2,
                _ => {}
            }
        }
        let mut dxac2 = 0.0;
        for (i, &xm) in xm[3..].iter().enumerate() {
            let arg = (xm * xm - 1.0).sqrt() / tanle;
            let (table, at) = if arg > 1.0 {
                (&cm::SUPAF2, 1.0 / arg)
            } else {
                (&cm::SUPAF1, arg)
            };
            self.tra[94] = ix(&cm::T422AF, 6, &[6, 6, 6], table, &[at, var2, tapr], flags)?;
            match i {
                0 => dxac2 = self.tra[94],
                1 => dxac2 = (self.tra[94] - dxac2) / 0.2,
                _ => {}
            }
        }
        let xmv: [f32; 6] = self.tra[83..89].try_into().expect("six Machs");
        let mut xacv: [f32; 6] = self.tra[89..95].try_into().expect("six centres");
        let mach = self.mach;
        let mut xac = fair(&xmv, &mut xacv, dxac1, dxac2, Some(0.0), mach);
        if tovc > 0.07 {
            let cosc2 = self.a[73];
            let delxac = ix(
                &cm::T428,
                7,
                &[4, 7],
                &cm::D428,
                &[tovc * 100.0, ar * (cosc2 * cosc2)],
                [0, -1, 0, 0, 1, 0, 0, 0],
            )?;
            self.tra[96] = delxac;
            let mfb = self.tra[6];
            let mut zmt = [
                0.6,
                (mfb + 0.6) / 2.0,
                mfb,
                mfb + 0.03,
                mfb + 0.07,
                mfb + 0.14,
                xmv[4],
                1.4,
            ];
            if zmt[5] + 0.01 >= zmt[6] {
                zmt[6] = (zmt[5] + 1.4) / 2.0;
            }
            for (n, &z) in zmt.iter().enumerate() {
                self.tra[97 + n] = fair(&xmv, &mut xacv, dxac1, dxac2, Some(0.0), z);
            }
            xac = fair(
                &zmt,
                &mut self.tra[97..105],
                dxac1,
                dxac2,
                Some(delxac),
                mach,
            );
        }
        self.tra[105] = xac;
        let xacw = xac * self.a[10] / self.cbarr;
        self.tra[95] = xacw;
        self.wing[121] = (self.a[173] / self.cbarr - xacw) * self.wing[101];
        Ok(())
    }

    /// WINGCL: CL from EXPR ALPOW/ALPLW (linear to ALPLW, then a power law
    /// up to alpha(CLmax)), drag due to lift from FIG 4.1.5.2-55 and CLB
    /// interpolated between the pre-pass's Mach 0.6 and 1.4 ratios.
    fn wingcl(&mut self) -> Result<()> {
        let n = self.alphas.len();
        let (ao, als) = (self.alpow, self.alplw);
        let cla = self.wing[101];
        if ao != UNUSED && als != UNUSED {
            let clref = (als - ao) * cla;
            if self.tra[37] < als || self.tra[38] < clref {
                self.tra[37] = als;
                self.tra[38] = clref;
            }
            let (aclmax, clmax) = (self.tra[37], self.tra[38]);
            // Only used above ALPLW, so unreachable when ALPLW = ACLMAX.
            let (mut expn, mut a2, a1, mut a0) = (0.0, 0.0, 0.0, 0.0);
            if als != aclmax {
                expn = cla * (als - aclmax) / (cla * (als - ao) - clmax);
                a2 = cla * (als - ao) - clmax;
                a0 = clmax;
            }
            for i in 1..=n {
                let alpha = self.b[i + 22];
                if alpha > aclmax {
                    break;
                }
                let cl = &mut self.wing[i + 20];
                // An exact UNUSED test below ALPLW: WBTRA's -UNUSED stays.
                if alpha - als <= 0.0 {
                    if *cl == UNUSED {
                        *cl = cla * (alpha - ao);
                    }
                } else if cl.abs() == UNUSED {
                    *cl = a0
                        + a1 * (alpha - aclmax)
                        + a2 * powr((alpha - aclmax) / (als - aclmax), expn);
                }
            }
        }
        let (toc, mach) = (self.ain[16], self.mach);
        let tc13 = powr(toc, 1.0 / 3.0);
        let tc23 = powr(toc, 2.0 / 3.0);
        let var = [(mach * mach - 1.0) / tc23, self.a[7] * tc13, self.a[27]];
        let flags = [1, 1, 1, 0, 1, 1, 1, 0];
        let cdlr1 = ix(
            &w::PARM,
            7,
            &[7, 6, 4],
            &dep55(&w::DEP55A, 0x2041_3535),
            &var,
            flags,
        )?;
        let cdlr2 = ix(
            &w::PARM,
            7,
            &[7, 6, 4],
            &dep55(&w::DEP55B, 0x2042_3535),
            &var,
            flags,
        )?;
        let x = self.a[7] * self.a[35].tan();
        let cdlcl2 = tb(&[0.0, 3.0], &[cdlr1, cdlr2], x, 1, 1)? * tc13;
        self.second[17] = cdlcl2;
        if self.wing[2].abs() == UNUSED {
            let cdo = self.wing[1];
            for i in 1..=n {
                let cl = self.wing[i + 20];
                if cl.abs() != UNUSED {
                    self.wing[i] = cdlcl2 * (cl * cl) + cdo;
                }
            }
        }
        let (clbsb, clbss) = (self.second[1], self.second[2]);
        let (cla6, cla14) = (self.tra[70], self.tra[1]);
        let sub = clbsb / (cla6 * cla6);
        let clbcl = ((clbss / (cla14 * cla14) - sub) * (mach - 0.6) / 0.8 + sub) * (cla * cla);
        self.second[18] = clbcl;
        for i in 1..=n {
            let cl = self.wing[i + 20];
            if cl.abs() != UNUSED && self.wing[i + 180].abs() == UNUSED {
                self.wing[i + 180] = clbcl * cl;
            }
        }
        Ok(())
    }

    /// CNCA's wing part: CMA and CLA as alpha derivatives of the CM and CL
    /// curves, and body-axis CN/CA where CL and CD are set.
    fn cnca(&mut self) -> Result<()> {
        let n = self.alphas.len();
        let alp = self.alphas.clone();
        let hold = Limits {
            lower: lim(0),
            upper: lim(0),
        };
        for j in 1..=n {
            let (sa, ca) = ((alp[j - 1] / RAD).sin(), (alp[j - 1] / RAD).cos());
            if self.wing[j + 40].abs() != UNUSED && self.wing[42].abs() != UNUSED {
                let curve = &self.wing[41..41 + n];
                self.wing[j + 120] =
                    interp::tbfunx_unordered(&alp, curve, alp[j - 1], hold)?.derivative;
            }
            let flag = self.wing[j + 20].abs() != UNUSED;
            if flag && j >= 2 {
                let curve = &self.wing[21..21 + n];
                self.wing[j + 100] =
                    interp::tbfunx_unordered(&alp, curve, alp[j - 1], hold)?.derivative;
            }
            if flag && self.wing[j].abs() != UNUSED {
                self.wing[j + 60] = self.wing[j + 20] * ca + self.wing[j] * sa;
                self.wing[j + 80] = self.wing[j] * ca - self.wing[j + 20] * sa;
            }
        }
        Ok(())
    }
}
