//! Body station geometry: EQSPCE/GETMAX and geometric integrals from BODYJM.
//! Aerodynamic nose/afterbody corrections remain with BODYRT in checkpoint 7.

use super::{GeometryError, Result};
use crate::{
    input::{Namelist, UNUSED},
    interp, math,
    tables::PI,
};

#[derive(Debug, Clone)]
pub struct BodyStations {
    pub x: Vec<f32>,
    pub radius: Vec<f32>,
    pub perimeter: Vec<f32>,
    pub area: Vec<f32>,
}

#[derive(Debug, Clone)]
pub struct EquallySpacedBody {
    pub stations: BodyStations,
    pub area_derivative: Vec<f32>,
}

#[derive(Debug, Clone)]
pub struct BodyGeometry {
    pub stations: BodyStations,
    pub equally_spaced: EquallySpacedBody,
    pub length: f32,
    pub base_area: f32,
    pub maximum_area: f32,
    /// GETMAX keeps the first of equally large stations.
    pub maximum_area_station: usize,
    pub equivalent_radius: Vec<f32>,
    pub volume: f32,
    pub projected_area: f32,
    pub projected_centroid: f32,
}

pub(super) fn station_count(body: &Namelist) -> Result<usize> {
    let nx = body
        .scalar("NX")
        .ok_or_else(|| GeometryError::Invalid("BODY.NX is required".into()))?;
    let n = (nx + 0.5).trunc();
    if !n.is_finite() || !(1.0..=20.0).contains(&n) {
        return Err(GeometryError::Invalid(
            "BODY.NX must select 1..20 stations".into(),
        ));
    }
    Ok(n as usize)
}

fn validate(stations: &BodyStations) -> Result<()> {
    let n = stations.x.len();
    if n == 0
        || stations.radius.len() != n
        || stations.perimeter.len() != n
        || stations.area.len() != n
    {
        return Err(GeometryError::Invalid(
            "body station arrays must have equal nonzero lengths".into(),
        ));
    }
    for values in [
        &stations.x,
        &stations.radius,
        &stations.perimeter,
        &stations.area,
    ] {
        math::finite(values)?;
        if values.contains(&UNUSED) {
            return Err(GeometryError::Invalid(
                "body station data is incomplete".into(),
            ));
        }
    }
    if stations.x.windows(2).any(|w| w[1] <= w[0])
        || stations
            .radius
            .iter()
            .chain(&stations.perimeter)
            .chain(&stations.area)
            .any(|&v| v < 0.0)
    {
        return Err(GeometryError::Invalid(
            "body X must increase and R/S/P must be nonnegative".into(),
        ));
    }
    Ok(())
}

/// EQSPCE: linear interpolation with GLOOK knot snapping, then TBFUNX slopes.
pub fn eqspce(stations: &BodyStations, count: usize) -> Result<EquallySpacedBody> {
    validate(stations)?;
    if !(2..=20).contains(&count) {
        return Err(GeometryError::Invalid(
            "EQSPCE requires 2..20 output stations".into(),
        ));
    }
    let n = stations.x.len();
    let mut result = BodyStations {
        x: vec![stations.x[0]; count],
        radius: vec![stations.radius[0]; count],
        perimeter: vec![stations.perimeter[0]; count],
        area: vec![stations.area[0]; count],
    };
    let mut derivative = vec![0.0; count];
    if n > 1 {
        let step = (stations.x[n - 1] - stations.x[0]) / (count - 1) as f32;
        result.x[count - 1] = stations.x[n - 1];
        result.radius[count - 1] = stations.radius[n - 1];
        result.perimeter[count - 1] = stations.perimeter[n - 1];
        result.area[count - 1] = stations.area[n - 1];
        for i in 1..count - 1 {
            result.x[i] = result.x[i - 1] + step;
            for (src, dst) in [
                (&stations.radius, &mut result.radius),
                (&stations.perimeter, &mut result.perimeter),
                (&stations.area, &mut result.area),
            ] {
                dst[i] = interp::interx(
                    &[&stations.x],
                    src,
                    &[result.x[i]],
                    &[interp::Limits::default()],
                )?
                .value;
            }
        }
        for (i, slope) in derivative.iter_mut().enumerate() {
            *slope = interp::tbfunx(
                &result.x,
                &result.area,
                result.x[i],
                interp::Limits::default(),
            )?
            .derivative;
        }
    }
    Ok(EquallySpacedBody {
        stations: result,
        area_derivative: derivative,
    })
}

pub(super) fn from_namelist(body: &Namelist) -> Result<BodyGeometry> {
    let n = station_count(body)?;
    let array = |name| body.get(name).expect("BODY schema")[..n].to_vec();
    body_geometry(BodyStations {
        x: array("X"),
        radius: array("R"),
        perimeter: array("P"),
        area: array("S"),
    })
}

pub fn body_geometry(stations: BodyStations) -> Result<BodyGeometry> {
    validate(&stations)?;
    let n = stations.x.len();
    let equivalent_radius: Vec<f32> = stations.area.iter().map(|&s| (s / PI).sqrt()).collect();
    let rx: Vec<f32> = equivalent_radius
        .iter()
        .zip(&stations.x)
        .map(|(&r, &x)| r * x)
        .collect();
    let volume = math::frustum_volume(&stations.x, &equivalent_radius)?;
    let integral = *math::trapz(&stations.x, &rx)?
        .last()
        .expect("nonempty stations");
    let projected_area = 2.0
        * math::trapz(&stations.x, &equivalent_radius)?
            .last()
            .expect("nonempty stations");
    // A point-only or zero-width body has no defined projected centroid.
    if projected_area <= 0.0 {
        return Err(GeometryError::Invalid(
            "body projected area must be positive".into(),
        ));
    }
    let projected_centroid = 2.0 * integral / projected_area;
    math::finite(&[volume, projected_area, projected_centroid])?;
    let mut maximum_area_station = 0;
    for i in 1..n {
        if stations.area[i] > stations.area[maximum_area_station] {
            maximum_area_station = i;
        }
    }
    let equally_spaced = eqspce(&stations, 20)?;
    Ok(BodyGeometry {
        length: stations.x[n - 1],
        base_area: stations.area[n - 1],
        maximum_area: stations.area[maximum_area_station],
        maximum_area_station,
        stations,
        equally_spaced,
        equivalent_radius,
        volume,
        projected_area,
        projected_centroid,
    })
}
