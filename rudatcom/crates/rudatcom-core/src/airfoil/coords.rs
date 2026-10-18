//! Section coordinate generators: ports of COORD1, COORD4, COORD5, COORD6,
//! CORD4M, CORD5M, CORDSP, XYCORD, SLEQ and ARCCOS.
//!
//! Expressions keep Fortran's left-to-right f32 evaluation; `x**2/3/4` are
//! plain products and real exponents go through `powr` (libm powf).

use super::{powr, AirfoilError, Result, Section, SlotView, NP};
use crate::{
    input::Case,
    interp::{tbfunx, Limits},
    tables::{PI, RAD},
};

fn sq(x: f32) -> f32 {
    x * x
}
fn cube(x: f32) -> f32 {
    x * x * x
}
fn quart(x: f32) -> f32 {
    let s = x * x;
    s * s
}

/// NACA 4-digit thickness distribution (also used by the 5-digit family).
fn thickness4(t: f32, x: f32) -> f32 {
    5.0 * t
        * (0.2969 * x.sqrt() - 0.126 * x - 0.3516 * sq(x) + 0.2843 * cube(x) - 0.1015 * quart(x))
}

/// Read a variable the original may use before assigning (stale locals).
fn defined(value: Option<f32>, name: &str) -> Result<f32> {
    value.ok_or_else(|| {
        AirfoilError::Invalid(format!(
            "NACA section {name} is undefined at a grid point (original uses an unset variable)"
        ))
    })
}

/// ARCCOS: arc cosine built from ATAN; |A| > 1 gives the inverse cosh.
pub(super) fn arccos(a: f32) -> f32 {
    if a == 0.0 {
        PI / 2.0
    } else if a.abs() <= 1.0 {
        let x = ((1.0 - sq(a)).sqrt() / a).atan();
        if x < 0.0 {
            PI + x
        } else {
            x
        }
    } else {
        (a + (sq(a) - 1.0).sqrt()).abs().ln()
    }
}

/// SLEQ: Gauss-Jordan solve of `n` equations. `a` is the column-major
/// N x (N+1) work array; a zero pivot rotates the rows down and restarts.
fn sleq(a: &mut [f32], b: &[f32], n: usize) -> Result<Vec<f32>> {
    let np1 = n + 1;
    let at = |i: usize, j: usize| (i - 1) + (j - 1) * n;
    for k in 1..=n {
        a[at(k, np1)] = b[k - 1];
    }
    let mut ind = 0;
    'restart: loop {
        for k in 1..=n {
            if a[at(k, k)] == 0.0 {
                ind += 1;
                if ind > n {
                    // The original prints "SLEQ ERRORS" and returns X unset.
                    return Err(AirfoilError::Invalid(
                        "SLEQ ERRORS: singular NACA thickness equations".into(),
                    ));
                }
                for col in 1..=np1 {
                    let temp = a[at(n, col)];
                    for j in 1..n {
                        a[at(n - j + 1, col)] = a[at(n - j, col)];
                    }
                    a[at(1, col)] = temp;
                }
                continue 'restart;
            }
            for j in k + 1..=np1 {
                a[at(k, j)] /= a[at(k, k)];
            }
            a[at(k, k)] = 1.0;
            for i in 1..=n {
                if i == k {
                    continue;
                }
                for j in k + 1..=np1 {
                    a[at(i, j)] -= a[at(i, k)] * a[at(k, j)];
                }
                a[at(i, k)] = 0.0;
            }
        }
        return Ok((1..=n).map(|l| a[at(l, np1)]).collect());
    }
}

/// Coefficients of the leading-edge (A0..A3, x < ZT) and trailing-edge
/// (D0..D3, x > ZT) thickness polynomials shared by the 1-, 6- and modified
/// families.
struct ThicknessPoly {
    zt: f32,
    t: f32,
    a: [f32; 4],
    d: [f32; 4],
}

impl ThicknessPoly {
    /// Solve for D2, D3 (TE curvature) then A1..A3 (LE fit) as the original.
    fn solve(t: f32, zt: f32, d0: f32, d1: f32, a0: f32) -> Result<Self> {
        let mut a = [
            -2.0 * (1.0 - zt),
            sq(1.0 - zt),
            -3.0 * sq(1.0 - zt),
            cube(1.0 - zt),
            0.0,
            0.0,
        ];
        let b = [d1, t / 2.0 - d0 - d1 * (1.0 - zt)];
        let xn = sleq(&mut a, &b, 2)?;
        let (d2, d3) = (xn[0], xn[1]);
        let mut aa = [
            0.0,
            1.0,
            zt,
            2.0,
            2.0 * zt,
            sq(zt),
            6.0 * zt,
            3.0 * sq(zt),
            cube(zt),
            0.0,
            0.0,
            0.0,
        ];
        let ba = [
            2.0 * d2 + 6.0 * d3 * (1.0 - zt) + a0 / (4.0 * powr(zt, 1.5)),
            -(a0 / (2.0 * powr(zt, 0.5))),
            -(a0 * powr(zt, 0.5)) + t / 2.0,
        ];
        let xm = sleq(&mut aa, &ba, 3)?;
        Ok(Self {
            zt,
            t,
            a: [a0, xm[0], xm[1], xm[2]],
            d: [d0, d1, d2, d3],
        })
    }

    /// YT at x; None only for a NaN abscissa (all comparisons false).
    fn yt(&self, x: f32) -> Option<f32> {
        let [a0, a1, a2, a3] = self.a;
        let [d0, d1, d2, d3] = self.d;
        if x == self.zt {
            Some(self.t / 2.0)
        } else if x < self.zt {
            Some(a0 * powr(x, 0.5) + a1 * x + a2 * sq(x) + a3 * cube(x))
        } else if x > self.zt {
            Some(d0 + d1 * (1.0 - x) + d2 * sq(1.0 - x) + d3 * cube(1.0 - x))
        } else {
            None
        }
    }

    /// The modified families' D1 from the TE-angle table, by ZT window.
    fn modified_d1(zt: f32, t: f32) -> Result<f32> {
        let windows = [
            (0.18, 0.22, 1.0),
            (0.28, 0.32, 1.17),
            (0.38, 0.42, 1.575),
            (0.48, 0.52, 2.325),
            (0.58, 0.62, 3.5),
        ];
        // The windows are disjoint, so the first match is the only match.
        windows
            .iter()
            .find(|&&(lo, hi, _)| zt > lo && zt < hi)
            .map(|&(_, _, k)| k * t)
            .ok_or_else(|| {
                AirfoilError::Invalid(
                    "modified NACA section: position of maximum thickness must be 2-6 (tenths of chord)"
                        .into(),
                )
            })
    }
}

/// NACA a=1 uniform-load mean line (COORD1 and COORD6 with a=1.0).
fn uniform_mean_line(cli: f32, x: f32) -> (f32, f32) {
    let yc = -(cli / (4.0 * PI)) * ((1.0 - x) * (1.0 - x).ln() + x * x.ln());
    let alpha = ((-(cli / (4.0 * PI))) * (x.ln() - (1.0 - x).ln())).atan();
    (yc, alpha)
}

/// The 5-digit mean line's ZM: the cubic root for the design-lift location.
fn five_digit_zm(zp: f32) -> f32 {
    let a = 6.0 * zp - 3.0;
    let b = -2.0 + 6.0 * zp - 3.0 * sq(zp);
    let g = b * b / 4.0 + a * a * a / 27.0;
    if g < 0.0 {
        let phi = arccos((-(b / 2.0)) / powr(-(cube(a) / 27.0), 0.5));
        1.0 + 2.0 * powr(-(a / 3.0), 0.5) * (phi / 3.0 + 4.18879).cos()
    } else {
        let third = 1.0f32 / 3.0;
        let d = powr(-(b / 2.0) + powr(g, 0.5), third);
        let e = powr(-(b / 2.0) - powr(g, 0.5), third);
        d + e + 1.0
    }
}

/// 5-digit mean-line state: ordinate/slope carry over between grid points
/// when x == ZM (no branch assigns them), as in the original.
struct FiveDigitMean {
    ai: f32,
    zm: f32,
    zp: f32,
    xk: f32,
    reflexed: bool,
    yc: Option<f32>,
    alpha: Option<f32>,
}

impl FiveDigitMean {
    fn new(ai: f32, ak: f32, zm: f32, zp: f32) -> Self {
        let xk = 6.0 * ai * 0.01 / (cube(zp) - 3.0 * zm * sq(zp) + sq(zm) * (3.0 - zm) * zp);
        Self {
            ai,
            zm,
            zp,
            xk,
            reflexed: ak != 0.0,
            yc: None,
            alpha: None,
        }
    }

    /// Evaluate at x. `zp_before_cubic` is CORD5M's (unreflexed) statement
    /// order, where the x == ZP assignment can be overridden by x < ZM.
    fn eval(&mut self, x: f32, zp_before_cubic: bool) {
        let (zm, zp) = (self.zm, self.zp);
        let sixth = 1.0f32 / 6.0;
        if !self.reflexed {
            let xk = self.xk;
            if zp_before_cubic && x == zp {
                self.yc = Some(self.ai * 0.01);
                self.alpha = Some(0.0);
            }
            if x < zm {
                self.yc = Some(sixth * xk * (cube(x) - 3.0 * zm * sq(x) + sq(zm) * (3.0 - zm) * x));
                self.alpha =
                    Some((sixth * xk * (3.0 * sq(x) - 6.0 * zm * x + sq(zm) * (3.0 - zm))).atan());
            }
            if !zp_before_cubic && x == zp {
                self.yc = Some(self.ai * 0.01);
                self.alpha = Some(0.0);
            }
            if x > zm {
                self.yc = Some(sixth * xk * cube(zm) * (1.0 - x));
                self.alpha = Some((-(sixth * xk * cube(zm))).atan());
            }
        } else {
            // Reflexed mean line (third digit 1): XK is recomputed every point.
            let rk = (3.0 * sq(zm - zp) - cube(zm)) / cube(1.0 - zm);
            let zmx = self.ai * 0.01;
            self.xk =
                6.0 * zmx / (cube(zp - zm) - rk * cube(1.0 - zm) * zp - cube(zm) * zp + cube(zm));
            let xk = self.xk;
            if x < zm {
                self.yc = Some(
                    sixth * xk * (cube(x - zm) - rk * x * cube(1.0 - zm) - cube(zm) * x + cube(zm)),
                );
                self.alpha =
                    Some((sixth * xk * (3.0 * sq(x - zm) - rk * cube(1.0 - zm) - cube(zm))).atan());
            }
            if x == zp {
                self.yc = Some(self.ai * 0.01);
                self.alpha = Some(0.0);
            }
            if x > zm {
                self.yc = Some(
                    sixth
                        * xk
                        * (rk * cube(x - zm) - rk * x * cube(1.0 - zm) - x * cube(zm) + cube(zm)),
                );
                self.alpha = Some(
                    (sixth * xk * (3.0 * rk * sq(x - zm) - rk * cube(1.0 - zm) - cube(zm))).atan(),
                );
            }
        }
    }
}

impl Section {
    fn digit_f(&self, i: usize) -> f32 {
        self.digits[i] as f32
    }

    /// Store one generated point: surface coordinates rotated by the
    /// mean-line slope, camber (tiny or negative values zeroed) and thickness.
    fn store_point(&mut self, m: usize, yc: f32, yt: f32, alpha: f32) {
        let x = self.x[m];
        self.xu[m] = x - yt * alpha.sin();
        self.yun[m] = yc + yt * alpha.cos();
        self.xl[m] = x + yt * alpha.sin();
        self.yln[m] = yc - yt * alpha.cos();
        self.cam[m] = if yc < 1e-5 { 0.0 } else { yc };
        self.thn[m] = yt;
    }

    /// Closed leading/trailing edge shared by every generator. The LE
    /// assignments differ per routine and are made by the callers.
    fn close_edges(&mut self, l: usize) {
        self.thn[0] = 0.0;
        self.thn[l - 1] = 0.0;
        self.cam[0] = 0.0;
        self.cam[l - 1] = 0.0;
        self.xu[l - 1] = 1.0;
        self.yun[l - 1] = 0.0;
        self.xl[l - 1] = 1.0;
        self.yln[l - 1] = 0.0;
    }

    /// COORD4: NACA 4-digit section (MPTT).
    pub(super) fn coord4(&mut self) -> Result<()> {
        let [i, j, k, ii, jj, kk, ..] = std::array::from_fn::<f32, 10, _>(|n| self.digit_f(n));
        let zm = i * 0.01;
        let zp = j * 0.1;
        let t = k * 0.1 + ii * 0.01 + jj * 0.001 + kk * 0.0001;
        self.rho = 1.1019 * sq(t);
        self.tmax = t;
        self.covc = zm;
        self.cxvc = zp;
        let l = self.n;
        for m in 0..l {
            let x = self.x[m];
            let yt = thickness4(t, x);
            let (mut yc, mut alpha) = (None, None);
            if x == zp {
                yc = Some(zm);
                alpha = Some(0.0);
            }
            if x < zp {
                yc = Some((2.0 * zp * x - sq(x)) * zm / sq(zp));
                alpha = Some((2.0 * zm / sq(zp) * (zp - x)).atan());
            }
            if x > zp {
                yc = Some(zm / sq(1.0 - zp) * (1.0 - 2.0 * zp + 2.0 * zp * x - sq(x)));
                alpha = Some((2.0 * zm / sq(1.0 - zp) * (zp - x)).atan());
            }
            self.store_point(m, defined(yc, "mean line")?, yt, defined(alpha, "slope")?);
        }
        self.close_edges(l);
        self.xu[0] = 0.0;
        self.yln[0] = 0.0;
        Ok(())
    }

    /// CORD4M: NACA modified 4-digit section (MPTT-IT).
    pub(super) fn cord4m(&mut self) -> Result<()> {
        let [i, j, k, ii, _, kk, iii, ..] = std::array::from_fn::<f32, 10, _>(|n| self.digit_f(n));
        let zm = i * 0.01;
        let zp = j * 0.1;
        let zt = iii * 0.1;
        let t = k * 0.1 + ii * 0.01;
        let d1 = ThicknessPoly::modified_d1(zt, t)?;
        let d0 = 0.01 * t;
        // LE radius index I (KK) scales the radius by (I*T/6)^2.
        let a0 = (2.0f32 * 1.1019 * sq(t * kk / 6.0)).sqrt();
        self.rho = 0.5 * sq(a0);
        let poly = ThicknessPoly::solve(t, zt, d0, d1, a0)?;
        self.tmax = t;
        self.covc = zm;
        self.cxvc = zp;
        let l = self.n;
        for m in 0..l {
            let x = self.x[m];
            let (mut yc, mut alpha) = (None, None);
            if x == zp {
                yc = Some(zm);
                alpha = Some(0.0);
            }
            if x < zp {
                yc = Some(zm / sq(zp) * (2.0 * zp * x - sq(x)));
                alpha = Some((2.0 * zm / sq(zp) * (zp - x)).atan());
            }
            if x > zp {
                yc = Some(zm / sq(1.0 - zp) * (1.0 - 2.0 * zp + 2.0 * zp * x - sq(x)));
                alpha = Some((2.0 * zm / sq(1.0 - zp) * (zp - x)).atan());
            }
            let yt = defined(poly.yt(x), "thickness")?;
            self.store_point(m, defined(yc, "mean line")?, yt, defined(alpha, "slope")?);
        }
        self.close_edges(l);
        self.xu[0] = 0.0;
        self.yln[0] = 0.0;
        Ok(())
    }

    /// COORD5: NACA 5-digit section (LPQTT); Q=1 selects the reflexed line.
    pub(super) fn coord5(&mut self) -> Result<()> {
        let [i, j, k, ii, jj, kk, iii, ..] = std::array::from_fn::<f32, 10, _>(|n| self.digit_f(n));
        let t = ii * 0.1 + jj * 0.01 + kk * 0.001 + iii * 0.0001;
        self.rho = 1.1019 * sq(t);
        self.tmax = t;
        let zp = j * 0.1 / 2.0;
        let zm = five_digit_zm(zp);
        self.covc = zm;
        self.cxvc = zp;
        let mut mean = FiveDigitMean::new(i, k, zm, zp);
        let l = self.n;
        for m in 0..l {
            let x = self.x[m];
            let yt = thickness4(t, x);
            mean.eval(x, false);
            self.store_point(
                m,
                defined(mean.yc, "mean line")?,
                yt,
                defined(mean.alpha, "slope")?,
            );
        }
        self.close_edges(l);
        self.xu[0] = 0.0;
        self.yln[0] = 0.0;
        Ok(())
    }

    /// CORD5M: NACA modified 5-digit section (LPQTT-IT).
    pub(super) fn cord5m(&mut self) -> Result<()> {
        let [i, j, k, ii, jj, _, iii, jjj, ..] =
            std::array::from_fn::<f32, 10, _>(|n| self.digit_f(n));
        let t = ii * 0.1 + jj * 0.01;
        let zp = j * 0.1 / 2.0;
        let zt = jjj * 0.1;
        let zm = five_digit_zm(zp);
        self.covc = zm;
        self.cxvc = zp;
        let mut mean = FiveDigitMean::new(i, k, zm, zp);
        let d1 = ThicknessPoly::modified_d1(zt, t)?;
        let d0 = 0.01 * t;
        let a0 = (2.0f32 * 1.1019 * sq(t * iii / 6.0)).sqrt();
        self.rho = 0.5 * sq(a0);
        self.tmax = t;
        let poly = ThicknessPoly::solve(t, zt, d0, d1, a0)?;
        let l = self.n;
        for m in 0..l {
            let x = self.x[m];
            mean.eval(x, true);
            let yt = defined(poly.yt(x), "thickness")?;
            self.store_point(
                m,
                defined(mean.yc, "mean line")?,
                yt,
                defined(mean.alpha, "slope")?,
            );
        }
        self.close_edges(l);
        self.xu[0] = 0.0;
        self.yln[0] = 0.0;
        Ok(())
    }

    /// COORD1: NACA 1-series (16-series) section with an a=1 mean line.
    pub(super) fn coord1(&mut self) -> Result<()> {
        let [_, aj, _, aii, ajj, akk, aiii, ajjj, ..] =
            std::array::from_fn::<f32, 10, _>(|n| self.digit_f(n));
        let j = self.digits[1];
        let mut zt = aj * 0.1 - 0.1;
        let t = ajj * 0.1 + akk * 0.01 + aiii * 0.001 + ajjj * 0.0001;
        if j == 6 {
            zt = aj * 0.1 - 0.2;
        }
        let (d1, sm) = match j {
            6 => (2.157 * t, 4.0),
            8 => (3.6833 * t, 3.0),
            9 => (5.5283 * t, 3.0),
            // D1/SM would be undefined in the original.
            _ => {
                return Err(AirfoilError::Invalid(
                    "NACA 1-series section: second digit must be 6, 8 or 9".into(),
                ))
            }
        };
        let a0 = (2.0f32 * 1.1019 * sq(t * sm / 6.0)).sqrt();
        self.rho = 0.5 * sq(a0);
        self.tmax = t;
        let poly = ThicknessPoly::solve(t, zt, 0.0, d1, a0)?;
        let l = self.n;
        // CLI is the COMMON variable AII, rescaled from the digit.
        self.cli = aii * 0.1;
        for m in 1..l - 1 {
            let x = self.x[m];
            let (yc, alpha) = uniform_mean_line(self.cli, x);
            let yt = defined(poly.yt(x), "thickness")?;
            self.store_point(m, yc, yt, alpha);
        }
        self.close_edges(l);
        // XU(1)/YUN(1) are left from the previous section, as in the original.
        self.xl[0] = 0.0;
        self.yln[0] = 0.0;
        self.ai = 0.0;
        self.alo = -(RAD * self.cli / (2.0 * PI));
        Ok(())
    }

    /// COORD6: NACA 6-series section (6X(Y)-ZTT, A-modified, a=0.x lines).
    pub(super) fn coord6(&mut self) -> Result<()> {
        let [_, _, _, _, ajj, akk, aiii, ajjj, ..] =
            std::array::from_fn::<f32, 10, _>(|n| self.digit_f(n));
        let (j, ii) = (self.digits[1], self.digits[3]);
        let t = akk * 0.1 + aiii * 0.01;
        // Series 63/64/65 (else 66) TE thickness; 'A' sections (II>0) differ.
        let (zt, d1) = match j {
            3 => {
                let r = -0.6116 * (t - 0.06) + 0.46;
                (0.35, if ii > 0 { t } else { r * t })
            }
            4 => {
                let r = -0.6888 * (t - 0.06) + 0.523;
                (0.40, if ii > 0 { 1.04 * t } else { r * t })
            }
            5 => {
                let r = -0.8833 * (t - 0.06) + 0.65;
                (0.4, if ii > 0 { 1.17 * t } else { r * t })
            }
            _ => {
                let r = -1.268 * (t - 0.06) + 0.873;
                (0.45, r * t)
            }
        };
        let rle = 0.01 * (68.682 * sq(t) + 0.0182 * t + 0.0014);
        self.rho = rle;
        self.tmax = t;
        let a0 = (2.0 * rle).sqrt();
        let poly = ThicknessPoly::solve(t, zt, 0.0, d1, a0)?;
        // a=0.x mean line (JJJ = x); a < 0.1 means the uniform a=1 line.
        let mut za = ajjj * 0.1;
        if ajjj < 1.0 {
            za = 1.0;
        }
        let (mut c, mut g, mut h) = (0.0, 0.0, 0.0);
        if za != 1.0 {
            c = 1.0 - za;
            g = (-(1.0 / c)) * (sq(za) * (0.5 * za.ln() - 0.25) + 0.25);
            h = (1.0 / c) * (0.5 * sq(c) * c.ln() - 0.25 * sq(c)) + g;
        }
        self.cli = ajj * 0.1;
        let cli = self.cli;
        let l = self.n;
        // 'A' sections get straight surfaces aft of 80% chord (NO/SXU logic).
        let mut no: Option<i32> = None;
        let (mut sxu, mut sxl, mut syu, mut syl, mut smu, mut sml) = (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        for m in 1..l - 1 {
            let x = self.x[m];
            let (yc, alpha) = if za != 1.0 {
                let s = 1.0 - x;
                let mut d = za - x;
                if d == 0.0 {
                    d = 1.0e-10;
                }
                let yc = cli / (2.0 * PI * (za + 1.0))
                    * ((1.0 / c)
                        * (0.5 * sq(d) * d.abs().ln() - 0.5 * sq(s) * s.ln() + 0.25 * sq(s)
                            - 0.25 * sq(d))
                        - x * x.ln()
                        + g
                        - x * h);
                let alpha = (cli / (2.0 * PI * (1.0 + za))
                    * ((1.0 / c) * (-(d * d.abs().ln()) + s * s.ln()) - x.ln() - 1.0 - h))
                    .atan();
                (yc, alpha)
            } else {
                uniform_mean_line(cli, x)
            };
            let yt = defined(poly.yt(x), "thickness")?;
            self.store_point(m, yc, yt, alpha);
            if self.xu[m] >= 0.80 && ii > 0 {
                match no {
                    Some(1) => {
                        sxu = self.xu[m];
                        sxl = self.xl[m];
                        syu = self.yun[m];
                        syl = self.yln[m];
                        smu = -(syu / (1.0 - sxu));
                        sml = -(syl / (1.0 - sxl));
                        no = Some(2);
                    }
                    Some(_) => {
                        self.xu[m] -= sxu;
                        self.xl[m] -= sxl;
                        self.yun[m] = smu * self.xu[m] + syu;
                        self.yln[m] = sml * self.xl[m] + syl;
                        self.xu[m] += sxu;
                        self.xl[m] += sxl;
                    }
                    None => {
                        return Err(AirfoilError::Invalid(
                            "NACA 6-series 'A' section: no grid point ahead of 80% chord".into(),
                        ))
                    }
                }
            } else {
                no = Some(1);
            }
        }
        self.close_edges(l);
        self.xu[0] = 0.0;
        self.yln[0] = 0.0;
        self.xl[0] = 0.0;
        self.yun[0] = 0.0;
        self.ai = 0.0;
        self.alo = -(RAD * cli / (2.0 * PI));
        if za != 1.0 {
            self.ai = self.alo * h / (za + 1.0);
            self.alo += self.ai;
        }
        Ok(())
    }

    /// CORDSP: supersonic S-1 double wedge, S-2 biconvex or S-3 hexagonal
    /// section over all 60 stations; also presets the surface's thickness,
    /// KSHARP and zero leading-edge radii.
    pub(super) fn cordsp(&mut self, case: &mut Case, view: &SlotView) {
        let digits = self.digits;
        let d = |n: usize| digits[n] as f32;
        let mut xt = (100.0 * d(1) + 10.0 * d(2) + d(3)) / 1000.0;
        let toc = (100.0 * d(4) + 10.0 * d(5) + d(6)) / 1000.0;
        let xf = (100.0 * d(7) + 10.0 * d(8) + d(9)) / 1000.0;
        self.rho = 0.0;
        self.tmax = toc;
        let ksharp;
        match self.digits[0] {
            1 => {
                ksharp = (1.0 / xt) / (1.0 - xt);
                for il in 0..NP {
                    let x = self.x[il];
                    self.yun[il] = if x > xt {
                        toc / 2.0 - (x - xt) * toc / (2.0 * (1.0 - xt))
                    } else {
                        x * toc / (2.0 * xt)
                    };
                }
            }
            3 => {
                ksharp = (1.0 - xf) / (xt * (1.0 - xt - xf));
                for il in 0..NP {
                    let x = self.x[il];
                    self.yun[il] = if x >= xt + xf {
                        toc / 2.0 - (x - xt - xf) * toc / (2.0 * (1.0 - xt - xf))
                    } else if x >= xt {
                        toc / 2.0
                    } else {
                        x * toc / (2.0 * xt)
                    };
                }
            }
            // 2, and the computed GO TO's fall-through, is the biconvex arc.
            _ => {
                xt = 0.50;
                ksharp = 16.0f32 / 3.0;
                let rc = (sq(toc) + 1.0) / (4.0 * toc);
                for il in 0..NP {
                    let x = self.x[il];
                    self.yun[il] = toc / 2.0 - rc + (sq(rc) - sq(x - 0.5)).sqrt();
                }
            }
        }
        for il in 0..NP {
            self.xu[il] = self.x[il];
            self.xl[il] = self.x[il];
            self.yln[il] = -self.yun[il];
        }
        view.fill(case, 16, toc);
        view.fill(case, 18, xt);
        view.fill(case, 70, toc);
        view.fill(case, 71, ksharp);
        view.fill(case, 62, 0.0);
        // The original's xxIN(15) .NE. 'STRA' test is always true here.
        view.fill(case, 63, 0.0);
        self.yu = self.yun;
        self.yl = self.yln;
    }

    /// XYCORD: thickness/mean line from user coordinates (IVAL 0) or user
    /// mean line/thickness (IVAL 1), then surface coordinates normal to the
    /// mean line using TBFUNX slopes.
    pub(super) fn xycord(&mut self, ival: i32) -> Result<()> {
        let n = self.n;
        if ival == 0 {
            for i in 0..n {
                self.thn[i] = 0.5 * (self.yu[i] - self.yl[i]);
                self.cam[i] = 0.5 * (self.yu[i] + self.yl[i]);
            }
        }
        self.thn[0] = 0.0;
        self.thn[n - 1] = 0.0;
        self.cam[0] = 0.0;
        self.cam[n - 1] = 0.0;
        for i in 0..n {
            let dydx =
                tbfunx(&self.x[..n], &self.cam[..n], self.x[i], Limits::default())?.derivative;
            let theta = dydx.atan();
            let (sa, ca) = (theta.sin(), theta.cos());
            self.xu[i] = self.x[i] - self.thn[i] * sa;
            self.xl[i] = self.x[i] + self.thn[i] * sa;
            self.yun[i] = self.cam[i] + self.thn[i] * ca;
            self.yln[i] = self.cam[i] - self.thn[i] * ca;
        }
        self.xu[0] = 0.0;
        self.xl[0] = 0.0;
        self.xu[n - 1] = 1.0;
        self.xl[n - 1] = 1.0;
        self.yun[0] = 0.0;
        self.yun[n - 1] = 0.0;
        self.yln[0] = 0.0;
        self.yln[n - 1] = 0.0;
        Ok(())
    }
}
