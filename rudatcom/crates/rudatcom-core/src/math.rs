//! Ports of QUAD, QUADIN, TRAPZ, DET4, SIMUL4, AGENR and GDELTA.
//!
//! Preserve single precision and evaluation order; changing algorithms or using
//! fused multiply-add can change the downstream aerodynamic results.

use crate::{interp, tables::*};
use std::cell::Cell;

#[derive(Debug, Clone, PartialEq, thiserror::Error)]
#[error("{0}")]
pub struct MathError(pub &'static str);

thread_local! {
    /// Set while a computation whose NaN/Inf intermediates the original
    /// discards (MAIN03's transonic pre-pass) runs; see `tolerate_non_finite`.
    static TOLERATE: Cell<bool> = const { Cell::new(false) };
}

/// Run `f` with the non-finite checks disabled, so NaN/Inf propagate with
/// IEEE semantics as in the original. Only for passes whose non-finite
/// values never reach reported results.
pub(crate) fn tolerate_non_finite<T>(f: impl FnOnce() -> T) -> T {
    // Restores the previous state even if `f` panics.
    struct Restore(bool);
    impl Drop for Restore {
        fn drop(&mut self) {
            TOLERATE.with(|t| t.set(self.0));
        }
    }
    let _restore = Restore(TOLERATE.with(|t| t.replace(true)));
    f()
}

pub(crate) fn finite(values: &[f32]) -> Result<(), MathError> {
    if TOLERATE.with(Cell::get) || values.iter().all(|x| x.is_finite()) {
        Ok(())
    } else {
        Err(MathError("non-finite numerical input or result"))
    }
}

/// Raise `x` to a real power through libm `powf`, as gfortran -O0 does.
/// `black_box` stops LLVM rewriting e.g. `powf(x, 0.5)` as `sqrt(x)`, which
/// can differ in the last bit.
pub(crate) fn powr(x: f32, e: f32) -> f32 {
    x.powf(std::hint::black_box(e))
}

/// QUAD: quadratic value and derivative using the original determinant expansion.
pub fn quad(x: &[f32; 3], y: &[f32; 3], at: f32) -> Result<(f32, f32), MathError> {
    finite(x)?;
    finite(y)?;
    finite(&[at])?;
    let x2 = x.map(|v| v * v);
    let det = |a: [f32; 3], b: [f32; 3], c: [f32; 3]| {
        a[0] * (b[1] * c[2] - b[2] * c[1]) - b[0] * (a[1] * c[2] - a[2] * c[1])
            + c[0] * (a[1] * b[2] - a[2] * b[1])
    };
    let d = det([x2[0], x[0], 1.0], [x2[1], x[1], 1.0], [x2[2], x[2], 1.0]);
    if d == 0.0 {
        return Err(MathError("QUAD requires three distinct abscissas"));
    }
    let a = det([y[0], x[0], 1.0], [y[1], x[1], 1.0], [y[2], x[2], 1.0]) / d;
    let b = det([x2[0], y[0], 1.0], [x2[1], y[1], 1.0], [x2[2], y[2], 1.0]) / d;
    let c = det(
        [x2[0], x[0], y[0]],
        [x2[1], x[1], y[1]],
        [x2[2], x[2], y[2]],
    ) / d;
    let result = (at * (a * at + b) + c, 2.0 * a * at + b);
    finite(&[result.0, result.1])?;
    Ok(result)
}

/// QUADIN: five-point quadrature with a trapezoid, Simpson or 3/8 remainder.
/// Unlike the original's zero return for invalid steps, invalid input is explicit.
pub fn quadin(y: &[f32], step: f32) -> Result<f32, MathError> {
    finite(y)?;
    if y.is_empty() || !step.is_finite() || step <= 0.0 {
        return Err(MathError(
            "QUADIN requires samples and a positive finite step",
        ));
    }
    let n = y.len();
    let mut ans = 0.0;
    let mut k = n;
    if n >= 4 {
        for i in (0..n).step_by(4) {
            k = n - i;
            if k >= 5 {
                ans = ans
                    + 7.0 * y[i]
                    + 32.0 * y[i + 1]
                    + 12.0 * y[i + 2]
                    + 32.0 * y[i + 3]
                    + 7.0 * y[i + 4];
            }
        }
        ans = ans * step / 22.5;
    }
    ans = match k {
        1 => ans,
        2 => ans + step * (y[n - 1] + y[n - 2]) / 2.0,
        3 => ans + step * (y[n - 3] + 4.0 * y[n - 2] + y[n - 1]) / 3.0,
        4 => ans + 3.0 * step * (y[n - 4] + 3.0 * y[n - 3] + 3.0 * y[n - 2] + y[n - 1]) / 8.0,
        _ => unreachable!(),
    };
    finite(&[ans])?;
    Ok(ans)
}

/// TRAPZ: cumulative trapezoidal integrals, starting at zero.
pub fn trapz(x: &[f32], y: &[f32]) -> Result<Vec<f32>, MathError> {
    cumulative(x, y, false)
}

/// TRAPZ with NTEST < 0: conical-frustum volume from station radii.
pub fn frustum_volume(x: &[f32], radius: &[f32]) -> Result<f32, MathError> {
    let a = cumulative(x, radius, true)?;
    Ok(a[a.len() - 1])
}

fn cumulative(x: &[f32], y: &[f32], volume: bool) -> Result<Vec<f32>, MathError> {
    if x.is_empty() || x.len() != y.len() {
        return Err(MathError("TRAPZ requires nonempty arrays of equal length"));
    }
    finite(x)?;
    finite(y)?;
    let mut a = vec![0.0; x.len()];
    for i in 1..x.len() {
        let temp = if volume {
            PI / 3.0 * (x[i] - x[i - 1]) * (y[i] * y[i] + y[i - 1] * y[i - 1] + y[i] * y[i - 1])
        } else {
            0.5 * (x[i] - x[i - 1]) * (y[i] + y[i - 1])
        };
        a[i] = a[i - 1] + temp;
    }
    finite(&a)?;
    Ok(a)
}

/// DET4: determinant of a flat row-major 4x4 matrix.
pub fn det4(a: &[f32; 16]) -> Result<f32, MathError> {
    finite(a)?;
    let mut p = 0.0;
    for m in 0..4 {
        let mut minor = [0.0; 9];
        let mut k = 0;
        for (i, &value) in a.iter().enumerate().skip(4) {
            if i % 4 != m {
                minor[k] = value;
                k += 1;
            }
        }
        let mut pp = minor[0] * (minor[4] * minor[8] - minor[5] * minor[7])
            - minor[1] * (minor[3] * minor[8] - minor[5] * minor[6])
            + minor[2] * (minor[3] * minor[7] - minor[4] * minor[6]);
        if m == 1 || m == 3 {
            pp = -pp;
        }
        p += a[m] * pp;
    }
    finite(&[p])?;
    Ok(p)
}

/// SIMUL4: Cramer's rule, retaining DATCOM's row-major COFF storage.
pub fn simul4(a: &[f32; 16], rhs: &[f32; 4]) -> Result<[f32; 4], MathError> {
    finite(rhs)?;
    let d = det4(a)?;
    if d == 0.0 {
        return Err(MathError("SIMUL4 coefficient matrix is singular"));
    }
    let mut out = [0.0; 4];
    for m in 0..4 {
        let mut de = *a;
        for k in 0..4 {
            de[4 * k + m] = rhs[k];
        }
        out[m] = det4(&de)? / d;
    }
    finite(&out)?;
    Ok(out)
}

/// AGENR: coefficient matrix for flap spanwise loading; sweep is in degrees.
#[allow(clippy::approx_constant)] // Rounded coefficients are part of the original fit.
pub fn agenr(boak: &[f32; 4], sweep: f32) -> Result<[f32; 16], MathError> {
    finite(boak)?;
    finite(&[sweep])?;
    if boak.contains(&0.0) {
        return Err(MathError("AGENR span/chord ratios must be nonzero"));
    }
    let tan = (DEG * sweep).tan();
    let mut al = [0.0; 16];
    let mut a = [0.0; 16];
    for k in 0..4 {
        let boch = boak[k];
        let reciprocal = 1.0 / boch;
        let boc2 = boch * boch;
        let boctn = boch * tan;
        let denom = 1.0 + AGENR_C3[k] * (2.0 * boctn);
        if denom == 0.0 {
            return Err(MathError("AGENR has a zero sweep denominator"));
        }
        for i in 4 * k..4 * k + 4 {
            let tmp = (1.0 + AGENR_C1[i] * boctn).powi(2);
            al[i] = reciprocal / AGENR_C1[i] * ((tmp + boc2 * AGENR_C1[i].powi(2)).sqrt() - 1.0)
                - reciprocal / AGENR_C2[i]
                    * ((tmp + boc2 * AGENR_C2[i].powi(2)).sqrt() / denom - 1.0)
                - 2.0
                    * tan
                    * ((1.0 + AGENR_C3[k] * boctn).powi(2) + boc2 * AGENR_C3[k].powi(2)).sqrt()
                    / denom;
        }
        let j = 4 * k;
        let scale = -boch / 16.0;
        let base = |row: usize| {
            let f = -2.0 * AGENR_F[row][k];
            if row == k {
                -f
            } else {
                f
            }
        };
        a[j] = base(0)
            + scale
                * (2.6131 * al[j]
                    + 2.0 * (-0.70711 * al[j + 1] - 0.76537 * al[j + 2] + 0.20711 * al[j + 3]));
        a[j + 1] = base(1)
            + scale
                * (-1.4142 * al[j]
                    + 2.0 * (1.8478 * al[j + 1] - 0.50000 * al[j + 2] - 0.76537 * al[j + 3]));
        a[j + 2] = base(2)
            + scale
                * (1.0824 * al[j]
                    + 2.0 * (-1.2071 * al[j + 1] + 1.8478 * al[j + 2] - 0.70711 * al[j + 3]));
        a[j + 3] = base(3)
            + scale * (-0.5 * al[j] + 1.0824 * al[j + 1] - 1.4142 * al[j + 2] + 2.6131 * al[j + 3]);
    }
    finite(&a)?;
    Ok(a)
}

fn loading_solutions(boch: &[f32; 4], sweep: f32) -> Result<[f32; 16], MathError> {
    let a = agenr(boch, sweep)?;
    let mut gd = [0.0; 16];
    for i in 0..4 {
        let j = 4 * i;
        let rhs = [
            GDELTA_EQ[j],
            GDELTA_EQ[j + 1],
            GDELTA_EQ[j + 2],
            GDELTA_EQ[j + 3],
        ];
        gd[j..j + 4].copy_from_slice(&simul4(&a, &rhs)?);
    }
    Ok(gd)
}

/// GDELTA's symmetric branch: full-span and inboard/outboard loading curves.
#[derive(Debug, Clone)]
pub struct FlapLoading {
    /// Twelve fixed SPU stations followed by EFI and EFO.
    pub span: [f32; 14],
    pub full: [f32; 14],
    pub inboard: [f32; 14],
    pub outboard: [f32; 14],
}

pub fn gdelta(boch: &[f32; 4], sweep: f32, efi: f32, efo: f32) -> Result<FlapLoading, MathError> {
    finite(&[efi, efo])?;
    if !(0.0..=1.0).contains(&efi) || !(efi..=1.0).contains(&efo) {
        return Err(MathError(
            "GDELTA requires 0 <= inboard span <= outboard span <= 1",
        ));
    }
    let mut gd = loading_solutions(boch, sweep)?;
    let mut gi = [0.0; 12];
    for i in 0..3 {
        for j in 0..4 {
            for l in 0..4 {
                gi[4 * i + j] += GDELTA_CN[16 * i + 4 * j + l] * gd[4 * i + l];
            }
        }
    }
    for i in 0..4 {
        gd[4 * i..4 * i + 4].reverse();
    }
    let mut fgc = [0.0; 36];
    let fourth_curve = [gd[12], gd[13], gd[14], gd[15], 0.0];
    for l in (1..8).step_by(2) {
        fgc[27 + l] = interp::tbfunx(
            &GDELTA_SD,
            &fourth_curve,
            GDELTA_SPT[l],
            interp::Limits::default(),
        )?
        .value;
    }
    for curve in 0..4 {
        for j in 0..4 {
            let l = 4 * curve + j;
            let dst = 9 * curve + 2 * j;
            fgc[dst] = gd[l];
            if l < 12 {
                fgc[dst + 1] = gi[l];
            }
        }
        fgc[9 * curve + 7] = 0.25 * fgc[9 * curve + 6];
    }
    // ZRX's first column is zero; FGX overlays its remaining four columns.
    let mut zrx = [0.0; 60];
    for curve in 0..4 {
        for (l, &span) in GDELTA_SPAN.iter().enumerate() {
            zrx[12 + 12 * curve + l] = interp::tbfunx(
                &GDELTA_SPT,
                &fgc[9 * curve..9 * curve + 9],
                span,
                interp::Limits::default(),
            )?
            .value;
        }
    }
    let mut result = FlapLoading {
        span: [0.0; 14],
        full: [0.0; 14],
        inboard: [0.0; 14],
        outboard: [0.0; 14],
    };
    result.span[..12].copy_from_slice(&GDELTA_SPAN);
    result.span[12] = efi;
    result.span[13] = efo;
    for (l, &span) in result.span.iter().enumerate() {
        result.full[l] =
            interp::tbfunx(&GDELTA_SPT, &fgc[27..36], span, interp::Limits::default())?.value;
        result.inboard[l] = interp::tlin(
            &[&GDELTA_X, &GDELTA_SPAN],
            &zrx,
            &[efi, span],
            &[interp::Limits::default(); 2],
        )?
        .value;
        result.outboard[l] = interp::tlin(
            &[&GDELTA_X, &GDELTA_SPAN],
            &zrx,
            &[efo, span],
            &[interp::Limits::default(); 2],
        )?
        .value;
    }
    Ok(result)
}

/// GDELTA's ASYFP branch, with COMMON geometry supplied explicitly.
/// Returns GD1H..GD4H; no state is retained between calls.
pub fn gdelta_asymmetric(
    sweep: f32,
    tan_te: f32,
    tan_le: f32,
    semispan: f32,
    root_chord: f32,
) -> Result<[f32; 4], MathError> {
    finite(&[tan_te, tan_le, semispan, root_chord])?;
    let arg = (tan_te - tan_le) * semispan;
    let mut boch = [0.0; 4];
    for i in 0..4 {
        boch[i] = 2.0 * semispan / (root_chord + GDELTA_SD[3 - i] * arg);
    }
    let gd = loading_solutions(&boch, sweep)?;
    Ok([gd[15], gd[14], gd[13], gd[12]])
}
