//! Section aerodynamics: ports of ASMINT, DELY, IDEAL, SLOPE, THEORY and
//! MAXCL (Weber's method with compressibility, crest-critical Mach number and
//! the Datcom figure 4.1.1.4 CLmax charts).

use super::{powr, AirfoilError, Result, Section, NP, NW};
use crate::{
    input::UNUSED,
    interp::{interx, tbfunx, Limits},
    tables::{DEG, PI, RAD},
};

/// Weber collocation count as the REAL XN = 32. used in the formulas.
const XN: f32 = 32.0;
/// Safety cap on SLOPE's zero-lift-angle iteration (unbounded in Fortran).
const MAX_SLOPE_ITERATIONS: usize = 1000;

fn sq(x: f32) -> f32 {
    x * x
}

/// (-1.)**k: exactly +-1.
fn alt(k: i32) -> f32 {
    if k % 2 == 0 {
        1.0
    } else {
        -1.0
    }
}

/// ASMINT: smooth (cubic between, parabolic at the ends) interpolation of
/// `y(x)` at `at`, slopes averaged in angle between neighbouring chords.
pub(super) fn asmint(x: &[f32], y: &[f32], at: f32) -> Result<f32> {
    let npt = x.len();
    if npt < 4 {
        return Err(AirfoilError::Invalid(
            "ASMINT needs at least four section points".into(),
        ));
    }
    // 1-based accessors keep the Fortran index arithmetic readable.
    let xx = |i: usize| x[i - 1];
    let yy = |i: usize| y[i - 1];
    let (np1, np2) = (npt - 1, npt - 2);
    // LOCATE 1 (left end), 2 (interior interval j..k) or 3 (right end).
    let (j, k, locate) = if at < xx(2) {
        (2, 3, 1)
    } else if at > xx(np1) {
        (np1, npt, 3)
    } else {
        let mut found = None;
        let mut k = 0;
        for j in 2..=np2 {
            if at == xx(j) {
                return Ok(yy(j));
            }
            k = j + 1;
            if at < xx(k) {
                found = Some(j);
                break;
            }
        }
        match found {
            Some(j) => (j, j + 1, 2),
            None => return Ok(yy(k)),
        }
    };
    let mut yp = [0.0f32; 2];
    let passes = if locate == 2 { 2 } else { 1 };
    for (n, slot) in yp.iter_mut().enumerate().take(passes) {
        let (lp, mp, rp) = (j + n - 1, j + n, k + n);
        let sl = (yy(lp) - yy(mp)) / (xx(lp) - xx(mp));
        let sr = (yy(mp) - yy(rp)) / (xx(mp) - xx(rp));
        let angav = (sl.atan() + sr.atan()) / 2.0;
        *slot = angav.sin() / angav.cos();
    }
    let (a, b, c, d);
    if locate == 2 {
        let (xj, xk) = (xx(j), xx(k));
        let x1s = xj * xj;
        let x2s = xk * xk;
        let x12f = xj - xk;
        let y12f = yy(j) - yy(k);
        let x12s = x1s - x2s;
        let x12c = x1s * xj - x2s * xk;
        let y12p = yp[0] - yp[1];
        let red = 2.0 * xk * x12f - x12s;
        let grn = 3.0 * x2s * x12f - x12c;
        let yel = yp[1] * x12f - y12f;
        let e = 3.0 * x12s * red - 2.0 * x12f * grn;
        a = (y12p * red - 2.0 * x12f * yel) / e;
        b = (3.0 * x12s * yel - y12p * grn) / e;
        c = (y12f - a * x12c - b * x12s) / x12f;
        d = yy(k) - a * x2s * xk - b * x2s - c * xk;
    } else {
        // End parabola through the two end points with the inner point's slope.
        let (j, l) = if locate == 3 { (np1, np1) } else { (1, 2) };
        let k = j + 1;
        let z = (yy(j) - yy(k)) / (xx(j) - xx(k));
        a = 0.0;
        b = (yp[0] - z) / (2.0 * xx(l) - xx(j) - xx(k));
        c = yp[0] - 2.0 * b * xx(l);
        d = yy(j) - ((b * xx(j) + c) * xx(j));
    }
    Ok(((a * at + b) * at + c) * at + d)
}

// MAXCL figure tables (PARMnn axes at stride NX1, EVALnn X1-fastest).
const PARM58: [f32; 17] = [
    0., 1., 1.1, 1.25, 1.50, 2.00, 2.25, 2.50, 3.00, 3.50, 4.00, 4.50, 5.00, 0.3, 0.35, 0.4, 0.45,
];
const EVAL58: [f32; 52] = [
    0.8, 0.8, 0.81, 0.85, 0.98, 1.2, 1.31, 1.43, 1.58, 1.59, 1.55, 1.47, 1.42, //
    0.8, 0.8, 0.81, 0.85, 0.98, 1.2, 1.31, 1.43, 1.51, 1.52, 1.48, 1.41, 1.41, //
    0.8, 0.8, 0.81, 0.85, 0.98, 1.2, 1.31, 1.39, 1.44, 1.45, 1.43, 1.35, 1.35, //
    0.8, 0.8, 0.81, 0.85, 0.98, 1.2, 1.29, 1.33, 1.35, 1.35, 1.35, 1.35, 1.35,
];
const PARM59_DY: [f32; 10] = [0., 1., 1.5, 2., 2.5, 3., 3.5, 4., 4.5, 5.];
const PARM59_COVC: [f32; 4] = [0., 0.02, 0.04, 0.06];
const PARM59_CXVC: [f32; 4] = [0.15, 0.30, 0.40, 0.50];
const EVAL59: [f32; 160] = [
    0., 0., 0., 0., 0., 0., 0., 0., 0., 0., //
    0., 0.12, 0.23, 0.32, 0.23, 0.12, 0.07, 0.07, 0.06, 0.03, //
    0., 0.27, 0.36, 0.40, 0.30, 0.16, 0.1, 0.1, 0.1, 0.06, //
    0., 0.27, 0.36, 0.40, 0.30, 0.16, 0.1, 0.1, 0.1, 0.06, //
    0., 0., 0., 0., 0., 0., 0., 0., 0., 0., //
    0., 0., 0.17, 0.22, 0.25, 0.15, 0.08, 0.06, 0.08, 0.02, //
    0., 0.15, 0.27, 0.40, 0.33, 0.18, 0.08, 0.06, 0.07, 0.01, //
    0., 0.53, 0.64, 0.56, 0.38, 0.22, 0.10, 0.06, 0.06, 0., //
    0., 0., 0., 0., 0., 0., 0., 0., 0., 0., //
    0., 0., 0.05, 0.19, 0.23, 0.14, 0.07, 0.06, 0.06, 0.02, //
    0., 0.16, 0.27, 0.37, 0.34, 0.19, 0.10, 0.07, 0.09, 0.05, //
    0., 0.39, 0.50, 0.50, 0.40, 0.25, 0.13, 0.10, 0.13, 0.09, //
    0., 0., 0., 0., 0., 0., 0., 0., 0., 0., //
    0., 0., 0.04, 0.10, 0.15, 0.14, 0.09, 0.07, 0.10, 0.08, //
    0., 0.10, 0.19, 0.27, 0.30, 0.22, 0.15, 0.13, 0.18, 0.12, //
    0., 0.26, 0.36, 0.43, 0.45, 0.30, 0.21, 0.20, 0.23, 0.20,
];
const PARM60: [f32; 13] = [
    0., 0.25, 1., 1.75, 2., 2.75, 3., 3.5, 4., 4.5, 0.35, 0.4, 0.45,
];
const EVAL60: [f32; 30] = [
    0., 0.16, 0.19, 0.20, 0.18, 0.08, 0.05, -0.01, -0.03, -0.04, //
    0., 0.16, 0.19, 0.20, 0.18, 0.08, 0.05, 0.03, 0.04, 0.02, //
    0., 0.16, 0.19, 0.20, 0.18, 0.08, 0.08, 0.10, 0.12, 0.08,
];
const PARM61: [f32; 12] = [
    0., 1., 1.5, 2., 2.5, 3., 3.5, 4., 3.0e6, 6.0e6, 9.0e6, 2.5e7,
];
const EVAL61: [f32; 32] = [
    0., -0.09, -0.13, -0.13, -0.09, -0.11, -0.20, -0.24, //
    0., 0., -0.08, -0.07, -0.03, -0.03, -0.07, -0.10, //
    0., 0., 0., 0., 0., 0., 0., 0., //
    0., 0.12, 0.14, 0.07, 0.01, 0.11, 0.18, 0.20,
];
const PARM62: [f32; 8] = [0., 1., 1.5, 2., 2.5, 3., 3.5, 4.];
const EVAL62: [f32; 8] = [0., 0., -0.19, -0.37, -0.46, -0.5, -0.5, -0.5];

impl Section {
    /// DELY: the 6%-0.15% chord thickness difference (Datcom's delta-y).
    pub(super) fn dely(&mut self) -> Result<()> {
        let n = self.n;
        let at = |c: f32| tbfunx(&self.x[..n], &self.thn[..n], c, Limits::default());
        let yu15 = at(0.0015)?.value;
        let yu60 = at(0.0600)?.value;
        self.deltay = (yu60 - yu15) * 100.0;
        Ok(())
    }

    /// IDEAL: Weber source/vortex terms ST1..ST5 at 32 cosine-spaced points,
    /// then the ideal angle/lift (unless CLI was set by a 1- or 6-series
    /// generator), zero-lift angle and CM about the quarter chord.
    // 1-based index loops mirror the original's coupled I/J sums.
    #[allow(clippy::needless_range_loop)]
    pub(super) fn ideal(&mut self) -> Result<()> {
        let n = NW;
        let l = n - 1;
        let mut thmu = [0.0f32; NW + 1];
        let w = &mut self.weber;
        for (j, th) in thmu.iter_mut().enumerate().take(l + 1).skip(1) {
            *th = j as f32 * PI / XN;
        }
        for i in 1..=n {
            w.thnu[i] = i as f32 * PI / XN;
            w.x[i] = 0.5 * (w.thnu[i].cos() + 1.0);
        }
        self.a0 = (2.0 * self.rho).sqrt();
        let a0 = self.a0;
        let np = self.n;
        for i in 1..=n {
            let at = self.weber.x[i];
            self.weber.zt[i] = if self.col(8) == b'S' {
                // Supersonic sections: linear TBFUNX over all 60 stations.
                tbfunx(&self.x[..NP], &self.thn[..NP], at, Limits::default())?.value
            } else {
                asmint(&self.x[..np], &self.thn[..np], at)?
            };
        }
        let w = &mut self.weber;
        // ST1: source distribution in parallel flow; ST2: thickness slope.
        for i in 1..=n {
            let (si, ci) = (w.thnu[i].sin(), w.thnu[i].cos());
            w.st1[i] = 0.0;
            w.st2[i] = 0.0;
            for j in 1..=l {
                let (s1, s2) = if i == j {
                    (XN / si, ci / sq(si))
                } else {
                    let p = alt(j as i32 - i as i32);
                    let (sj, cj) = (thmu[j].sin(), thmu[j].cos());
                    let s1 = (p - 1.0) / XN * 2.0 * sj / sq(cj - ci);
                    let s2 = -(2.0 * p * sj / (si * (cj - ci)));
                    (s1, s2)
                };
                w.st2[i] += s2 * w.zt[j];
                w.st1[i] += s1 * w.zt[j];
            }
        }
        w.st1[n] += XN * a0;
        // ST3: vortex distribution in normal flow (plus the LE-radius term).
        for i in 1..=n {
            let (si, ci) = (w.thnu[i].sin(), w.thnu[i].cos());
            w.st3[i] = 0.0;
            for j in 1..=l {
                let s3 = if i == j {
                    XN / si
                } else {
                    let p = alt(j as i32 - i as i32);
                    let (sj, cj) = (thmu[j].sin(), thmu[j].cos());
                    (p - 1.0) / XN * 2.0 * sj / sq(cj - ci)
                        + 2.0 / XN * (1.0 - p) / (sj * (cj - ci))
                };
                w.st3[i] += s3 * w.zt[j];
            }
        }
        for i in 1..=n {
            let y = if i == n {
                XN / 2.0 * a0
            } else {
                let s3 = (alt(i as i32) - 1.0) / (XN * (1.0 + w.thnu[i].cos()));
                s3 * (self.rho / 2.0).sqrt()
            };
            w.st3[i] += y;
        }
        for i in 1..=n {
            let at = w.x[i];
            w.zc[i] = asmint(&self.x[..np], &self.cam[..np], at)?;
        }
        // ST4: vortex distribution due to camber; ST5: mean-line slope.
        for i in 1..=n {
            let (si, ci) = (w.thnu[i].sin(), w.thnu[i].cos());
            w.st4[i] = 0.0;
            w.st5[i] = 0.0;
            for j in 1..=l {
                let pj = alt(j as i32);
                let (s4, s5) = if i == j {
                    (
                        XN / si - 2.0 * (pj - 1.0) / (XN * si * (1.0 - ci)),
                        -(ci / sq(si)),
                    )
                } else {
                    let p = alt(j as i32 - i as i32);
                    let cj = thmu[j].cos();
                    let s4 = 2.0 * (p - 1.0) / (XN * si) * (1.0 - cj * ci) / sq(ci - cj)
                        - 2.0 * (pj - 1.0) / (XN * si * (1.0 - cj));
                    let s5 = -(2.0 * p / (cj - ci));
                    (s4, s5)
                };
                w.st4[i] += s4 * w.zc[j];
                w.st5[i] += s5 * w.zc[j];
            }
        }
        if self.cli == UNUSED {
            // Planform ideal parameters from the 5%/95% chord camber terms.
            let jn = n - 4;
            let half_sin = |i: usize| w.thnu[i].sin() / 2.0;
            let zc95 = w.zc[4];
            let zc05 = w.zc[jn];
            let dai05 = 0.3739 * zc05 + 0.04745 * w.st5[n - 1];
            let dai95 = -(0.3739 * zc95) + 0.04745 * w.st5[1];
            let da95 = -(0.7834 * zc95) + 0.09518 * w.st5[1];
            let t1 = 0.5
                * (-(zc95 * w.thnu[4].cos() / sq(half_sin(4)))
                    - zc05 * w.thnu[jn].cos() / sq(half_sin(jn)));
            let t2 = 0.5 * zc95 / (1.0 - w.thnu[4].cos());
            let nm5 = n - 5;
            let (mut sumal, mut sumai) = (0.0f32, 0.0f32);
            for i in 5..=l {
                if i <= nm5 {
                    sumai += -(w.zc[i] * w.thnu[i].cos() / sq(half_sin(i)));
                }
                sumal += w.zc[i] / (1.0 - w.thnu[i].cos());
            }
            let dai = 0.5 / XN * (t1 + sumai);
            self.ai = RAD * (dai05 + dai + dai95);
            let dalo = -(2.0 / XN * (t2 + sumal));
            self.alo = RAD * (da95 + dalo);
            self.cli = 2.0 * PI / RAD * (self.ai - self.alo);
        }
        let mut sumcm = 0.0f32;
        for i in 1..=l {
            sumcm += w.zc[i] * w.thnu[i].cos();
        }
        let xmuo = sumcm * (-(PI / XN));
        self.cmco4 = 2.0 * xmuo + PI / 2.0 * self.alo / RAD;
        Ok(())
    }

    /// SLOPE: Weber pressure integration at ALO and ALO+1 deg, iterating ALO
    /// to zero lift; returns (CLALPA, XAC). Updates ALO, A0 and, at Mach 0,
    /// CMCO4 and the crest-critical XC/MCC/CLCC. CLALPA stays UNUSED when
    /// M >= 1; a failed analysis warns and returns the slope reached so far.
    pub(super) fn slope(&mut self, mach: f32, renn: f32) -> Result<(f32, Option<f32>)> {
        let n = NW;
        let nm1 = n - 1;
        let mut clalpa = UNUSED;
        if mach >= 1.0 {
            return Ok((clalpa, None));
        }
        let w = self.weber.clone();
        let mut alpha = [self.alo, self.alo + 1.0];
        let beta = (1.0 - sq(mach)).sqrt();
        self.a0 = (2.0 * self.rho).sqrt();
        let a0 = self.a0;
        let tmach = if mach != 0.0 {
            1.0 / (0.7 * sq(mach))
        } else {
            0.0
        };
        let mut vu = [0.0f32; NW + 1];
        let mut vl = [0.0f32; NW + 1];
        let mut cpum = [0.0f32; NW + 1];
        let mut cplm = [0.0f32; NW + 1];
        let (mut cl, mut cm) = ([0.0f32; 2], [0.0f32; 2]);
        let (mut sina, mut cosa) = (0.0f32, 0.0f32);
        let mut iterations = 0;
        let failed = 'iterate: loop {
            for i in 0..2 {
                cosa = (alpha[i] * DEG).cos();
                sina = (alpha[i] * DEG).sin();
                for l in 1..=nm1 {
                    let cpi = 1.0 - sq(1.0 + w.st1[l]) / (1.0 + sq(w.st2[l]));
                    let bo = 1.0 - sq(mach) * (1.0 - mach * cpi);
                    if bo < 0.0 {
                        break 'iterate true;
                    }
                    let bo = bo.sqrt();
                    let root = ((1.0 - w.x[l]) / w.x[l]).sqrt();
                    vu[l] = (cosa * (1.0 + w.st1[l] / bo + w.st4[l] / beta)
                        + sina / beta * (1.0 + w.st3[l] / bo) * root)
                        / (1.0 + sq((w.st2[l] + w.st5[l]) / bo)).sqrt();
                    vl[l] = (cosa * (1.0 + w.st1[l] / bo - w.st4[l] / beta)
                        - sina / beta * (1.0 + w.st3[l] / bo) * root)
                        / (1.0 + sq((w.st2[l] - w.st5[l]) / bo)).sqrt();
                    if mach == 0.0 {
                        cpum[l] = 1.0 - sq(vu[l]);
                        cplm[l] = 1.0 - sq(vl[l]);
                    } else {
                        // Isentropic compressible pressure coefficient.
                        let cpuvd = 1.0 + 0.2 * sq(mach) * (1.0 - sq(vu[l]));
                        if cpuvd < 0.0 {
                            break 'iterate true;
                        }
                        let cplvd = 1.0 + 0.2 * sq(mach) * (1.0 - sq(vl[l]));
                        if cplvd < 0.0 {
                            break 'iterate true;
                        }
                        cpum[l] = tmach * (powr(cpuvd, 3.5) - 1.0);
                        cplm[l] = tmach * (powr(cplvd, 3.5) - 1.0);
                    }
                }
                // Leading-edge point.
                vu[n] = if a0 != 0.0 {
                    sina * ((1.0 + w.st3[n]) / (0.5 * a0))
                } else {
                    0.0
                };
                vl[n] = vu[n];
                let cpuvdn = 1.0 + 0.2 * sq(mach) * (1.0 - sq(vu[n] / beta));
                if cpuvdn < 0.0 {
                    break 'iterate true;
                }
                cpum[n] = tmach * (powr(cpuvdn, 3.5) - 1.0);
                if tmach == 0.0 {
                    cpum[n] = 1.0 - sq(vu[n]);
                }
                cplm[n] = cpum[n];
                let mut sumcl = 0.0f32;
                for l in 1..=nm1 {
                    let lx = -(cpum[l] - cplm[l]);
                    sumcl += lx * (w.thnu[l].sin() / 2.0);
                }
                cl[i] = sumcl * PI / (cosa * XN);
                let mut sumcm = 0.0f32;
                for l in 1..=nm1 {
                    let lx = cpum[l] - cplm[l];
                    sumcm += lx * (w.x[l] - 0.25) * w.thnu[l].sin() / 2.0;
                }
                cm[i] = sumcm * PI / XN;
            }
            if cl[0].abs() <= 0.001 {
                break false;
            }
            // Newton step on the zero-lift angle.
            clalpa = cl[1] - cl[0];
            self.alo -= cl[0] / clalpa;
            alpha = [self.alo, self.alo + 1.0];
            iterations += 1;
            if iterations > MAX_SLOPE_ITERATIONS {
                return Err(AirfoilError::Invalid(
                    "SLOPE: zero-lift angle iteration did not converge".into(),
                ));
            }
        };
        if failed {
            self.warnings.push(
                "COMPUTATIONS IN SUBROUTINE SLOPE HAVE FAILED. ANALYSIS FOR THIS AIRFOIL IMPOSSIBLE."
                    .into(),
            );
            if mach == 0.0 {
                // Unreachable for finite data (BO = 1 at M = 0); the original
                // would continue with an undefined viscous factor.
                return Err(AirfoilError::Invalid(
                    "SLOPE failed at Mach 0; crest-critical analysis impossible".into(),
                ));
            }
            return Ok((clalpa, None));
        }
        // Viscous correction factor from the trailing-edge angle (PHITE).
        let mut ren = 1.0e6;
        if renn >= 10.0 && self.mcc != 0.0 && mach != 0.0 {
            ren = renn;
        }
        if ren < 2.71828e5 {
            self.warnings.push(
                "REYNOLDS NUMBER TOO LOW FOR THE AIRFOIL SECTION MODULE, SECTION CHARACTERISTICS BASED ON A VALUE OF 2.718E5"
                    .into(),
            );
            ren = 2.7182e5;
        }
        let phite = ((w.zt[6] + w.zt[7]) / 2.0 - w.zt[2]) / 0.089272624;
        let expn = -1.0 + 5.0 * phite / 2.0;
        let mut con =
            1.0 - powr((ren / 1.0e5).ln(), expn) * (0.232 + 1.785 * phite - 2.950 * sq(phite));
        if con < 0.6896 {
            con = 0.6896;
        }
        clalpa = (cl[1] - cl[0]) * con * 1.05;
        let cmalpa = cm[1] - cm[0];
        if mach == 0.0 {
            self.cmco4 = cm[0];
        }
        let xacm = 0.25 - cmalpa / clalpa;
        if mach != 0.0 {
            return Ok((clalpa, Some(xacm)));
        }
        // Crest-critical conditions (Mach 0 only), from the last alpha's
        // surface slope and velocity distributions.
        let clm = clalpa;
        let mut xi = [0.0f32; NW];
        let mut stnu = [0.0f32; NW];
        for i in 1..=nm1 {
            let j = n - i;
            xi[i] = w.x[j];
            stnu[i] = w.st2[j] + w.st5[j];
        }
        let stc = sina / cosa;
        let ii = (1..=nm1).find(|&i| stc > stnu[i]).unwrap_or(nm1);
        if ii == 1 {
            return Err(AirfoilError::Invalid(
                "crest-critical point lies ahead of the first Weber station".into(),
            ));
        }
        self.xc = (stc - stnu[ii]) / (stnu[ii - 1] - stnu[ii]) * (xi[ii - 1] - xi[ii]) + xi[ii];
        let xc = self.xc;
        let ii = (1..=nm1).find(|&i| xc > w.x[i]).unwrap_or(nm1);
        if ii == 1 {
            return Err(AirfoilError::Invalid(
                "crest-critical point lies aft of the last Weber station".into(),
            ));
        }
        let vovu = (xc - w.x[ii]) / (w.x[ii - 1] - w.x[ii]) * (vu[ii - 1] - vu[ii]) + vu[ii];
        let cpcc = 1.0 - sq(vovu);
        let mcc = 1.023
            - 0.9507 * cpcc
            - 0.4140 * sq(cpcc)
            - 0.1506 * (sq(cpcc) * cpcc)
            - 0.0212 * sq(sq(cpcc));
        self.mcc = 1.0 / mcc;
        let bcc = (1.0 - sq(self.mcc)).sqrt();
        self.clcc = (clm.abs() / (bcc - (sq(self.mcc) / (1.0 + bcc)) * clm.abs() / 2.0)) * con;
        Ok((clalpa, Some(xacm)))
    }

    /// THEORY: ideal properties, the Mach-0 slope, then CLALPA/XAC at each
    /// flight Mach number (CLCC at and beyond the crest-critical Mach).
    pub(super) fn theory(&mut self, fltcon: &[f32]) -> Result<()> {
        self.ideal()?;
        let (cla0, _) = self.slope(0.0, 0.0)?;
        self.cla0 = cla0;
        let nmach = ((fltcon[0] + 0.5) as i32).clamp(0, 20) as usize;
        let mut crit = false;
        let mut failed = Vec::new();
        for i in 0..nmach {
            let mach = fltcon[i + 2];
            let renn = fltcon[i + 42] * self.cbar;
            if mach >= self.mcc {
                crit = true;
            }
            if self.cla[i] != UNUSED {
                continue;
            }
            if mach >= self.mcc {
                self.cla[i] = self.clcc;
            }
            if crit {
                continue;
            }
            let (cla, xac) = self.slope(mach, renn)?;
            self.cla[i] = cla;
            if let Some(xac) = xac {
                self.xac[i] = xac;
            }
            if cla == UNUSED {
                failed.push(mach);
            }
        }
        if !failed.is_empty() {
            // The original prints one message per Mach number and then STOPs.
            let list: Vec<String> = failed.iter().map(|m| format!("{m:.3}")).collect();
            return Err(AirfoilError::Invalid(format!(
                "AIRFOIL MODULE UNABLE TO CALCULATE SECTION CLALPA FOR MACH NO = {}; \
                 a value must be specified by the user (CLALPA and CLMAX in the xxSCHR namelist)",
                list.join(", ")
            )));
        }
        Ok(())
    }

    /// MAXCL: section CLmax = base (delta-y, x/c of max thickness) + camber
    /// increments + Reynolds-number increment, at each flight Mach number.
    pub(super) fn maxcl(&mut self, fltcon: &[f32]) -> Result<()> {
        let hold2 = [Limits::default(); 2];
        if (self.xovc - 0.3).abs() <= 1.0e-5 {
            self.xovc = 0.3;
        }
        let clbase = interx(
            &[&PARM58[..13], &PARM58[13..]],
            &EVAL58,
            &[self.deltay, self.xovc],
            &hold2,
        )?
        .value;
        let (mut del1, mut del2) = (0.0f32, 0.0f32);
        if self.camber {
            del1 = interx(
                &[&PARM59_DY, &PARM59_COVC, &PARM59_CXVC],
                &EVAL59,
                &[self.deltay, self.covc, self.cxvc],
                &[Limits::default(); 3],
            )?
            .value;
            if self.xovc != 0.30 {
                del2 = interx(
                    &[&PARM60[..10], &PARM60[10..]],
                    &EVAL60,
                    &[self.deltay, self.xovc],
                    &hold2,
                )?
                .value;
            }
        }
        let nmach = ((fltcon[0] + 0.5) as i32).clamp(0, 20) as usize;
        for i in 0..nmach {
            let rn = fltcon[i + 42] * self.cbar;
            let var2 = if rn < 1.0 { 9.0e6 } else { rn };
            let del3 = interx(
                &[&PARM61[..8], &PARM61[8..]],
                &EVAL61,
                &[self.deltay, var2],
                &hold2,
            )?
            .value;
            // DEL4 (figure 4.1.3.4-?) is evaluated but unused by the original.
            tbfunx(&PARM62, &EVAL62, self.deltay, Limits::default())?;
            self.clmax0 = clbase + del1 + del2 + del3;
            self.clmax[i] = self.clmax0;
        }
        Ok(())
    }
}
