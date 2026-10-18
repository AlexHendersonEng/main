//! Ports of GLOOK/SWITCH, TBFUNX, TLIN1X/TLINEX/TLIN3X/TLIN4X and INTERX.
//!
//! TBFUNX is piecewise linear in value but quadratic in derivative. TLIN uses
//! GLOOK's 0.1% knot snapping; replacing it with a conventional interpolator
//! changes results. Invalid/duplicate axes are errors instead of legacy NaNs.

use crate::math::{finite, quad, MathError};

#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub enum Extrapolation {
    /// Original flag 0: hold the endpoint and report extrapolation.
    #[default]
    Hold,
    /// Original negative flag: hold without a report in the TLIN family.
    HoldSilent,
    Linear,
    Quadratic,
}

#[derive(Debug, Clone, Copy, Default)]
pub struct Limits {
    /// Applies before the first stored knot, even for descending axes.
    pub lower: Extrapolation,
    /// Applies beyond the last stored knot, even for descending axes.
    pub upper: Extrapolation,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Side {
    BeforeFirst,
    AfterLast,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Extrapolated {
    pub axis: usize,
    pub side: Side,
    /// Whether the legacy TLIN/SWITCH policy requests an extrapolation message.
    pub report: bool,
}

#[derive(Debug, Clone)]
pub struct Interpolated {
    pub value: f32,
    pub extrapolated: Vec<Extrapolated>,
}

#[derive(Debug, Clone)]
pub struct CurvePoint {
    pub value: f32,
    pub derivative: f32,
    pub extrapolated: Option<Side>,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Lookup {
    /// Exact/snapped knot, or the second stored knot of the bounding interval.
    pub index: usize,
    /// None means a knot was selected without interpolation.
    pub fraction: Option<f32>,
}

fn validate_axis(axis: &[f32]) -> Result<(), MathError> {
    finite(axis)?;
    if axis.is_empty() {
        return Err(MathError("interpolation requires a nonempty axis"));
    }
    let ascending = axis[0] <= axis[axis.len() - 1];
    if axis.windows(2).any(|w| {
        if ascending {
            w[1] <= w[0]
        } else {
            w[1] >= w[0]
        }
    }) {
        return Err(MathError(
            "interpolation axes must be strictly monotonic (no duplicate knots)",
        ));
    }
    Ok(())
}

fn side(axis: &[f32], at: f32) -> Option<Side> {
    let ascending = axis[0] <= axis[axis.len() - 1];
    if (ascending && at < axis[0]) || (!ascending && at > axis[0]) {
        Some(Side::BeforeFirst)
    } else if (ascending && at > axis[axis.len() - 1]) || (!ascending && at < axis[axis.len() - 1])
    {
        Some(Side::AfterLast)
    } else {
        None
    }
}

fn lookup(axis: &[f32], at: f32) -> Lookup {
    let ascending = axis[0] <= axis[axis.len() - 1];
    let mut previous = 0.0;
    for (i, &x) in axis.iter().enumerate() {
        let delta = at - x;
        let mut denominator = if at == 0.0 { x } else { at };
        if denominator.abs() <= 0.0001 {
            denominator = 1.0;
        }
        if (delta / denominator).abs() < 1e-3 {
            return Lookup {
                index: i,
                fraction: None,
            };
        }
        if (ascending && delta < 0.0) || (!ascending && delta > 0.0) {
            return Lookup {
                index: i,
                fraction: (i != 0).then(|| previous / (previous - delta)),
            };
        }
        previous = delta;
    }
    Lookup {
        index: axis.len() - 1,
        fraction: None,
    }
}

/// GLOOK: ascending/descending lookup with the original relative knot tolerance.
pub fn glook(axis: &[f32], at: f32) -> Result<Lookup, MathError> {
    validate_axis(axis)?;
    finite(&[at])?;
    let result = lookup(axis, at);
    if let Some(fraction) = result.fraction {
        finite(&[fraction])?;
    }
    Ok(result)
}

/// TBFUNX: linear values and three-point quadratic derivatives.
///
/// For one sample the derivative is zero. Two samples always define a line,
/// regardless of extrapolation flags, as in the original.
pub fn tbfunx(
    axis: &[f32],
    values: &[f32],
    at: f32,
    limits: Limits,
) -> Result<CurvePoint, MathError> {
    validate_axis(axis)?;
    if axis.windows(2).any(|w| w[0] >= w[1]) {
        return Err(MathError(
            "TBFUNX requires equal lengths and an ascending axis",
        ));
    }
    tbfunx_unordered(axis, values, at, limits)
}

/// TBFUNX without the ascending-axis check, for curves the original feeds
/// with locally reversed abscissae (airfoil surface x near a cambered
/// leading edge). Like the original it brackets with the last knot <= `at`;
/// coincident knots make QUAD singular and are reported as non-finite.
pub fn tbfunx_unordered(
    axis: &[f32],
    values: &[f32],
    at: f32,
    limits: Limits,
) -> Result<CurvePoint, MathError> {
    finite(axis)?;
    finite(values)?;
    finite(&[at])?;
    if axis.is_empty() || values.len() != axis.len() {
        return Err(MathError(
            "TBFUNX requires equal lengths and an ascending axis",
        ));
    }
    let n = axis.len();
    let outside = side(axis, at);
    let (value, derivative) = if n == 1 {
        (values[0], 0.0)
    } else if n == 2 {
        let slope = (values[1] - values[0]) / (axis[1] - axis[0]);
        (values[0] + slope * (at - axis[0]), slope)
    } else if at > axis[0] && at < axis[n - 1] {
        let l = (0..n - 1).rfind(|&i| at >= axis[i]).unwrap_or(0).max(1);
        let xx = [axis[l - 1], axis[l], axis[l + 1]];
        let yy = [values[l - 1], values[l], values[l + 1]];
        let i = if at < axis[1] { 0 } else { 1 };
        let value = yy[i] + (yy[i + 1] - yy[i]) * (at - xx[i]) / (xx[i + 1] - xx[i]);
        (value, quad(&xx, &yy, at)?.1)
    } else {
        let upper = at >= axis[n - 1];
        let start = if upper { n - 3 } else { 0 };
        let xx = [axis[start], axis[start + 1], axis[start + 2]];
        let yy = [values[start], values[start + 1], values[start + 2]];
        let policy = if upper { limits.upper } else { limits.lower };
        match policy {
            Extrapolation::Hold | Extrapolation::HoldSilent => {
                (values[if upper { n - 1 } else { 0 }], quad(&xx, &yy, at)?.1)
            }
            Extrapolation::Quadratic => quad(&xx, &yy, at)?,
            Extrapolation::Linear => {
                // With exactly three points LE == NP3 on either side. The
                // original then lets LEXU=1 override the lower-end line too.
                let use_upper = limits.upper == Extrapolation::Linear && start == n - 3;
                let i = if use_upper { n - 2 } else { 0 };
                let slope = (values[i + 1] - values[i]) / (axis[i + 1] - axis[i]);
                let anchor = if use_upper { n - 1 } else { 0 };
                (values[anchor] + (at - axis[anchor]) * slope, slope)
            }
        }
    };
    finite(&[value, derivative])?;
    Ok(CurvePoint {
        value,
        derivative,
        extrapolated: outside,
    })
}

/// TLIN family for one to four axes in the original argument order.
///
/// Flat storage is X2-fastest, then X1, X3, X4 (TLINEX's Y(NX2,NX1)).
/// For ordinary X1-fastest figure tables, use `interx` instead.
pub fn tlin(
    axes: &[&[f32]],
    values: &[f32],
    at: &[f32],
    limits: &[Limits],
) -> Result<Interpolated, MathError> {
    let mut order: Vec<usize> = (0..axes.len()).collect();
    if order.len() > 1 {
        order.swap(0, 1);
    }
    interpolate(axes, values, at, limits, &order)
}

/// INTERX dispatcher in normal Fortran column-major order (X1 fastest).
///
/// The original's four-variable dispatch is commented out, so it is explicitly
/// unsupported here; TLIN4X remains available via `tlin`.
pub fn interx(
    axes: &[&[f32]],
    values: &[f32],
    at: &[f32],
    limits: &[Limits],
) -> Result<Interpolated, MathError> {
    if axes.len() > 3 {
        return Err(MathError(
            "original INTERX does not implement four-variable interpolation; use TLIN4X",
        ));
    }
    interpolate(
        axes,
        values,
        at,
        limits,
        &(0..axes.len()).collect::<Vec<_>>(),
    )
}

fn interpolate(
    axes: &[&[f32]],
    values: &[f32],
    at: &[f32],
    limits: &[Limits],
    order: &[usize],
) -> Result<Interpolated, MathError> {
    if axes.is_empty() || axes.len() > 4 || axes.len() != at.len() || axes.len() != limits.len() {
        return Err(MathError(
            "TLIN requires one to four axes with matching queries and limits",
        ));
    }
    let mut count = 1usize;
    let mut extrapolated = Vec::new();
    for (i, axis) in axes.iter().enumerate() {
        validate_axis(axis)?;
        count = count
            .checked_mul(axis.len())
            .ok_or(MathError("interpolation table size overflow"))?;
        if let Some(side) = side(axis, at[i]) {
            let policy = if side == Side::BeforeFirst {
                limits[i].lower
            } else {
                limits[i].upper
            };
            if axis.len() == 1 && matches!(policy, Extrapolation::Linear | Extrapolation::Quadratic)
            {
                return Err(MathError("extrapolation requires at least two knots"));
            }
            extrapolated.push(Extrapolated {
                axis: i,
                side,
                report: policy != Extrapolation::HoldSilent,
            });
        }
    }
    finite(at)?;
    finite(values)?;
    if count != values.len() {
        return Err(MathError(
            "dependent table length does not match axis dimensions",
        ));
    }
    let value = evaluate(axes, values, at, limits, order)?;
    finite(&[value])?;
    Ok(Interpolated {
        value,
        extrapolated,
    })
}

fn evaluate(
    axes: &[&[f32]],
    values: &[f32],
    at: &[f32],
    limits: &[Limits],
    order: &[usize],
) -> Result<f32, MathError> {
    let Some((&index, inner)) = order.split_last() else {
        return Ok(values[0]);
    };
    let axis = axes[index];
    let x = at[index];
    let stride: usize = inner.iter().map(|&i| axes[i].len()).product();
    let sample = |i| {
        evaluate(
            axes,
            &values[i * stride..(i + 1) * stride],
            at,
            limits,
            inner,
        )
    };
    if let Some(side) = side(axis, x) {
        let policy = if side == Side::BeforeFirst {
            limits[index].lower
        } else {
            limits[index].upper
        };
        if matches!(policy, Extrapolation::Linear | Extrapolation::Quadratic) {
            let upper = side == Side::AfterLast;
            if policy == Extrapolation::Quadratic && axis.len() > 2 {
                let i = if upper { axis.len() - 3 } else { 0 };
                return Ok(quad(
                    &[axis[i], axis[i + 1], axis[i + 2]],
                    &[sample(i)?, sample(i + 1)?, sample(i + 2)?],
                    x,
                )?
                .0);
            }
            let i = if upper { axis.len() - 2 } else { 0 };
            let t = (x - axis[if upper { i + 1 } else { i }]) / (axis[i + 1] - axis[i]);
            let y0 = sample(i)?;
            let y1 = sample(i + 1)?;
            return Ok(if upper {
                y1 + t * (y1 - y0)
            } else {
                y0 + t * (y1 - y0)
            });
        }
    }
    let found = lookup(axis, x);
    let y = sample(found.index)?;
    if let Some(t) = found.fraction {
        let previous = sample(found.index - 1)?;
        Ok(previous + t * (y - previous))
    } else {
        Ok(y)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn invalid_tables_fail_explicitly() {
        for axis in [&[][..], &[0.0, 0.0], &[0.0, 2.0, 1.0], &[f32::NAN]] {
            assert!(glook(axis, 0.0).is_err());
        }
        assert!(tbfunx(&[0.0, 1.0], &[1.0], 0.0, Limits::default()).is_err());
        assert!(tlin(&[&[0.0, 1.0]], &[1.0], &[0.0], &[Limits::default()]).is_err());
        assert!(interx(&[&[0.0][..]; 4], &[1.0], &[0.0; 4], &[Limits::default(); 4]).is_err());
        assert!(glook(&[0.0, 1.0], f32::INFINITY).is_err());
    }

    #[test]
    fn extrapolation_is_returned_to_the_caller() {
        let r = tlin(&[&[0.0, 1.0]], &[2.0, 3.0], &[-1.0], &[Limits::default()]).unwrap();
        assert_eq!(r.value, 2.0);
        assert_eq!(
            r.extrapolated,
            [Extrapolated {
                axis: 0,
                side: Side::BeforeFirst,
                report: true
            }]
        );
        let r = tlin(
            &[&[1.0, 0.0]],
            &[2.0, 3.0],
            &[2.0],
            &[Limits {
                lower: Extrapolation::HoldSilent,
                ..Limits::default()
            }],
        )
        .unwrap();
        assert!(!r.extrapolated[0].report);
    }
}
