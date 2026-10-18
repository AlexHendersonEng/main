//! M15O17: wing zero-lift angle (CALCA0), lift-curve slope and maximum lift
//! (WTLIFT, CLMXBS) and the lift curve (LIFTCF), DATCOM sections 4.1.3.
//! Statements follow the Fortran order so f32 rounding matches.

use super::{
    angles,
    data::{calca0 as t0, clmxbs as t2, liftcf as t3, wtlift as t1},
    tb, tl, WingCase, CRAN, CURV, DOUB, STRA,
};
use crate::input::UNUSED;
use crate::{
    aero::Result,
    tables::{DEG, PI, RAD},
};

// CALCA0's camber curves (FIG 4.1.3.1-?): points, CX and CY start (1-based).
const NPT: [usize; 7] = [7, 6, 4, 4, 3, 4, 5];
const LOCX: [usize; 7] = [1, 3, 5, 6, 8, 8, 8];
const LOCY: [usize; 7] = [1, 8, 14, 18, 22, 25, 29];

/// CALCA0's bracketing scan over a table axis. Returns the stopping index
/// II (1-based), the last and previous differences, and whether the loop
/// hit a knot within 0.02 or ran off the end (the NOINTx flags). Ascending
/// axes stop at the first knot above `x`, descending ones below.
fn scan(axis: &[f32], x: f32, descending: bool) -> (usize, f32, f32, bool) {
    let (mut temp, mut tempp) = (0.0, 0.0);
    for (i, &k) in axis.iter().enumerate() {
        temp = x - k;
        if temp.abs() < 2e-2 {
            return (i + 1, temp, tempp, true);
        }
        if (!descending && temp < 0.0) || (descending && temp > 0.0) {
            return (i + 1, temp, tempp, false);
        }
        tempp = temp;
    }
    (axis.len(), temp, tempp, true)
}

impl WingCase {
    /// CALCA0: zero-lift angle A(134) (WGSCHR ALPHAO in WINGIN(10) when
    /// given, else ALPHAI - CLI/CLalpha),
    /// the twist correction (FIG 4.1.3.1-4, DA0OT) and, for cambered
    /// sections, the Mach correction giving B(49).
    pub(super) fn calca0(&mut self) -> Result<()> {
        let (a, ain) = (&mut self.a, &self.ain);
        a[134] = if ain[10] != UNUSED {
            ain[10]
        } else {
            ain[20] - ain[19] / a[131]
        };
        if ain[11].abs() >= 0.5 {
            let (ii, temp, tempp, exact) = scan(&t0::TR, a[27], false);
            let nointp = exact || ii == 1;
            let mut itr = 3 * (ii - 1);
            let trr = if nointp { 0.0 } else { tempp / (tempp - temp) };
            let (mut iar, temp2, tempp2, exact2) = scan(&t0::AR, a[7], false);
            let noint2 = exact2 || iar == 1;
            let arr = if noint2 {
                0.0
            } else {
                tempp2 / (tempp2 - temp2)
            };
            let (ii, temp3, tempp3, exact3) = scan(&t0::SAQC, a[40], false);
            let noint3 = exact3 || ii == 1;
            let sar = if noint3 {
                0.0
            } else {
                tempp3 / (tempp3 - temp3)
            };
            if iar == 4 && a[7] <= 0.5 {
                iar = 3;
            }
            // DA0OT(22,10), column-major: sweep row, (taper, aspect) column.
            let da = |row: usize, col: usize| t0::DA0OT[(row - 1) + 22 * (col - 1)];
            // Sweep-interpolated table value at one column.
            let at = |col: usize| {
                let d = da(ii, col);
                if noint3 {
                    d
                } else {
                    da(ii - 1, col) + sar * (d - da(ii - 1, col))
                }
            };
            let mut second = false;
            let mut dsv = 0.0;
            // Label 1120: aspect-ratio blend, then a second pass at the
            // lower taper ratio for the taper blend.
            let d2 = loop {
                let idx = itr + iar;
                let mut d2 = at(idx);
                if !noint2 {
                    let d1 = at(idx - 1);
                    d2 = d1 + arr * (d2 - d1);
                }
                if nointp {
                    break d2;
                }
                if second {
                    break d2 + trr * (dsv - d2);
                }
                second = true;
                itr -= 3;
                dsv = d2;
                if iar == 4 {
                    iar = 3;
                }
            };
            a[135] = d2;
            a[134] += ain[11] * a[135];
        }
        if !self.camber {
            self.b[49] = a[134];
            return Ok(());
        }
        let (ii, temp, tempp, exact) = scan(&t0::TOC, 100.0 * ain[16], true);
        let nointp = exact || ii == 1 || temp < 0.0;
        let xag = a[43] * self.b[1];
        let curve = |k: usize| {
            let (np, x, y) = (NPT[k - 1], LOCX[k - 1] - 1, LOCY[k - 1] - 1);
            tb(&t0::CX[x..x + np], &t0::CY[y..y + np], xag, -1, 0)
        };
        let mut yag1 = curve(ii)?;
        if !nointp {
            let fact = tempp / (tempp - temp);
            let ysv = yag1;
            yag1 = curve(ii - 1)?;
            yag1 = yag1 + (ysv - yag1) * fact;
        }
        self.b[49] = yag1 * a[134];
        a[137] = yag1;
        Ok(())
    }

    /// WTLIFT: Helmbold lift-curve slopes of the inboard/outboard panels
    /// (A(172)/A(171), non-straight planforms), wing CLalpha WING(101), and
    /// CLmax B(44) with its angle B(43) by the high or low aspect-ratio
    /// method. Curved planforms get no CLalpha (message only).
    pub(super) fn wtlift(&mut self) -> Result<()> {
        let (a, b, ain) = (&mut self.a, &mut self.b, &self.ain);
        let sref = self.sref;
        let loarat = a[7] < a[125];
        let temp4 = b[2] * b[2];
        if self.kind != STRA {
            let temp1 = 2.0 * PI * a[168] * DEG;
            let temp2 = a[168] * DEG * 2.0 * PI / a[131];
            let temp3 = a[98] * a[98];
            a[172] = temp1 / (2.0 + (temp2 * temp2 * (1.0 + temp3 / temp4) + 4.0).sqrt());
            let temp1 = 2.0 * PI * a[5] * DEG;
            let temp2 = a[5] * DEG * 2.0 * PI / a[131];
            let temp3 = a[74] * a[74];
            a[171] = temp1 / (2.0 + (temp2 * temp2 * (1.0 + temp3 / temp4) + 4.0).sqrt());
        }
        if self.kind == CURV {
            self.messages.push(format!(
                "NO CLALPHA COMPUTATION- WING IS CURV LOW ASPECT RATIO= {}",
                if loarat { "T" } else { "F" }
            ));
            return Ok(());
        }
        a[159] = tb(&t1::TR, &t1::C2, a[27], 0, 0)?;
        a[160] = (a[159] + 1.0) * a[38] * a[7];
        let temp1 = 2.0 * PI * a[7] * DEG;
        let temp2 = a[7] * DEG * 2.0 * PI / a[131];
        let temp2 = temp2 * temp2;
        let temp3 = a[50] * a[50];
        let flags = [-1, 0, -1, 2];
        a[145] = tl(
            &[&t1::DELTAY, &t1::SALE],
            &t1::CLL,
            &[ain[17], a[34]],
            &flags,
        )?;
        a[144] = tl(&[&t1::DY, &t1::SALE], &t1::DACLL, &[ain[17], a[34]], &flags)?;
        let w = &mut self.wing;
        w[101] = (temp1 / (2.0 + (temp2 * (1.0 + temp3 / temp4) + 4.0).sqrt())) * a[3] / sref;
        if self.kind == CRAN {
            let clrat = tb(&t1::BA, &t1::CLOVCL, a[7] * b[2], 2, 0)?;
            w[101] *= clrat;
        }
        if !loarat {
            let temp5 = tl(
                &[&t1::DYA, &t1::AMACH, &t1::SALE4],
                &t1::DCAR,
                &[ain[17], b[1], a[34]],
                &[0, -1, 0, 2, 2, 2],
            )?;
            a[146] = a[145] * a[132];
            b[44] = (a[145] * a[132] + temp5) * a[3] / sref;
            b[43] = b[44] / w[101] + b[49] + a[144];
            return Ok(());
        }
        let c1p1ac = a[7] * a[124] / b[2];
        let clsmax = self.clmxbs(c1p1ac)?;
        let (a, b) = (&self.a, &mut self.b);
        b[44] = clsmax;
        let aclbas = tb(&t1::C1ABCS, &t1::ACLMX, c1p1ac, 0, 0)?;
        let dacmax = if a[160] <= 4.5 {
            let tmp = a[7] * a[37] * (1.0 + 4.0 * (a[27] * a[27]));
            tl(
                &[&t1::ACLE, &t1::C1TABO],
                &t1::DACLO,
                &[tmp, a[160]],
                &[0, 0, 2, 0],
            )?
        } else {
            tl(
                &[&t1::DMN, &t1::C1TAB],
                &t1::DACL,
                &[b[1], a[160]],
                &[-1, 0, 2, 0],
            )?
        };
        b[43] = aclbas + dacmax;
        Ok(())
    }

    /// CLMXBS: low aspect-ratio CLmax (FIG 4.1.3.4-16A/B base value plus the
    /// 4.1.3.4-17A Mach increment), scaled to the reference area.
    pub(super) fn clmxbs(&self, bu4: f32) -> Result<f32> {
        let (a, b, ain) = (&self.a, &self.b, &self.ain);
        let dclmax = tl(
            &[&t2::AMN, &t2::C2A],
            &t2::DE,
            &[b[1], a[160]],
            &[-1, 2, 0, 2],
        )?;
        // BASE(19,12): columns 1-6 for XOVC <= 0.35, 7-12 beyond.
        let base = if ain[18] > 0.35 {
            &t2::CBASE[114..]
        } else {
            &t2::CBASE[..114]
        };
        let clbase = tl(
            &[&t2::DYAG, &t2::C1ABC],
            base,
            &[ain[17], bu4],
            &[0, 0, -1, 0],
        )?;
        Ok((clbase + dclmax) * a[3] / self.sref)
    }

    /// LIFTCF: CL and CN versus alpha by planform type (straight tapered:
    /// 4.1.3.3 nonlinear method; curved: FIG 4.1.3.3-58; double delta and
    /// cranked: FIG 4.1.3.3-56/57 tables).
    pub(super) fn liftcf(&mut self) -> Result<()> {
        match self.kind {
            STRA => self.liftcf_straight(),
            CURV => {
                let (a, b, ain) = (&self.a, &self.b, &self.ain);
                let sqbo2 = (ain[3] / a[29]).sqrt();
                for j in 1..=self.alphas.len() {
                    let cljosq = tb(&t3::A58, &t3::CLJ58, b[j + 22], 0, 1)?;
                    self.wing[j + 20] = sqbo2 * cljosq * a[3] / self.sref;
                    self.wing[j + 60] = self.wing[j + 20] / (DEG * b[j + 22]).cos();
                }
                Ok(())
            }
            DOUB | CRAN => self.liftcf_delta(),
            _ => {
                self.messages.push(format!(
                    "METHOD NOT APPLICABLE-TYPE= {} LOW AR= {}",
                    self.kind,
                    if self.a[7] < self.a[125] { "T" } else { "F" }
                ));
                Ok(())
            }
        }
    }

    fn liftcf_straight(&mut self) -> Result<()> {
        let sref = self.sref;
        let a = &mut self.a;
        let ajayc = 0.3
            * (a[123] + 1.0)
            * a[7]
            * a[37]
            * ((a[123] + 1.0) * (a[159] + 1.0) - (a[160] / 7.0).powi(3));
        let ajay = ajayc / self.b[2];
        a[147] = self.b[43] - self.b[49];
        angles(&mut a[147..153])?;
        let cna = self.wing[101] * RAD;
        let b = &mut self.b;
        b[45] = (b[44] / (a[149] * a[150]) - cna * a[150]) / a[149].abs();
        // CLMXBS at 4/beta, evaluated once per call (the original's NEWM).
        let mut high_ar_clmax = None;
        for j in 1..=self.alphas.len() {
            let cnaaj = if self.b[j + 22] < self.b[43] {
                self.a[153] = self.b[j + 22] - self.b[49];
                angles(&mut self.a[153..159])?;
                let rat = (self.a[157] / self.a[151]).abs();
                let dcaaj = tl(&[&t3::AJ, &t3::TRAT], &t3::DC, &[ajay, rat], &[2, 1, -1, 0])?;
                self.b[45] + dcaaj * self.a[3] / sref
            } else {
                let clsmax = if self.a[7] < self.a[125] {
                    self.b[44]
                } else if let Some(c) = high_ar_clmax {
                    c
                } else {
                    let c = self.clmxbs(4.0 / self.b[2])?;
                    high_ar_clmax = Some(c);
                    c
                };
                let (a, b) = (&mut self.a, &self.b);
                let temp = b[49] / (90.0 - b[43]);
                a[153] = b[j + 22] * (1.0 + temp) - 90.0 * temp;
                angles(&mut a[153..159])?;
                let rat = (a[151] / a[157]).abs();
                // RAT is +Inf at alpha 0 when alpha(CLmax) < 0 (MAIN03's
                // pre-pass); TBFUNX holds the table end for it like any
                // large value, and the NaN CL that follows is the original's.
                let tir_end = t3::TIR[t3::TIR.len() - 1];
                let dj = tb(&t3::TIR, &t3::D, rat.min(tir_end), 0, 0)?;
                // AR and AI share TIR's storage in the original.
                let cnaa90 = if a[7] > 1.0 {
                    tb(&t3::TIR, &t3::C90I, 1.0 / a[7], 0, 0)? * a[3] / sref
                } else {
                    tb(&t3::TIR, &t3::C90, a[7], 0, 0)?
                };
                let ratio = b[2] * b[44] / clsmax;
                b[45]
                    + (cnaa90 - b[45]) * (1.0 - rat)
                    + RAD * dj * self.wing[101] / 2.3 * (ratio * ratio)
            };
            let (a, w) = (&self.a, &mut self.wing);
            w[j + 60] = (RAD * w[101] * a[156] + cnaaj * a[155].abs()) * a[155];
            w[j + 20] = w[j + 60] * (DEG * self.b[j + 22]).cos();
        }
        Ok(())
    }

    /// Double-delta and cranked planforms. Cranked wings are linear up to
    /// the break angle, then follow the double-delta curve offset to it.
    fn liftcf_delta(&mut self) -> Result<()> {
        let (a, ain) = (&self.a, &self.ain);
        let con = a[23] / (ain[3] * a[5]) * RAD;
        let btanle = self.b[2] * a[62];
        let ltabsw = !(a[7] >= 3.0 || self.b[1] < 0.7 || btanle > 7.0);
        let a7 = a[7];
        // CL/(CLalpha CON) from FIG 4.1.3.3-56: the 2-D table (first `rows`
        // X13356 rows) at large alpha or outside the 3-D table's range.
        let curve = |alpha: f32, rows: usize| {
            if alpha >= 12.0 || !ltabsw {
                tl(
                    &[&t3::X13356[..rows], &t3::X23356],
                    &t3::Y13356[..rows * 12],
                    &[alpha, btanle],
                    &[2, 2, 2, 1],
                )
            } else {
                tl(
                    &[&t3::X33356, &t3::X23356[..8], &t3::X13356[..4]],
                    &t3::Y33356,
                    &[a7, btanle, alpha],
                    &[2, 2, 2, 2, 1, 2],
                )
            }
        };
        let n = self.alphas.len();
        if self.kind == DOUB {
            for j in 1..=n {
                // B(J+22) is temporarily made positive, as in the original.
                let alpha = if self.b[j + 22] < 0.0 {
                    self.b[j + 22]
                } else {
                    0.0
                };
                self.b[j + 22] = self.b[j + 22].abs();
                let c = curve(self.b[j + 22], 6)?;
                self.wing[j + 20] = c * self.wing[101] * con;
                if alpha < 0.0 {
                    self.b[j + 22] = alpha;
                    self.wing[j + 20] = -self.wing[j + 20];
                }
                self.wing[j + 60] = self.wing[j + 20] / (DEG * self.b[j + 22]).cos();
            }
            return Ok(());
        }
        let abreak = tb(&t3::X13357, &t3::Y13357, 1.0 / a[62], 0, 2)?;
        let clbrak = self.wing[101] * abreak;
        let clabnl = curve(abreak, 5)? * self.wing[101] * con;
        for j in 1..=n {
            let alpha = self.b[j + 22];
            let w = &mut self.wing;
            if alpha <= abreak {
                w[j + 20] = w[101] * alpha;
            } else {
                w[j + 20] = curve(alpha, 6)? * w[101] * con;
                w[j + 20] = clbrak + w[j + 20] - clabnl;
            }
            w[j + 60] = w[j + 20] / (DEG * alpha).cos();
        }
        Ok(())
    }
}
