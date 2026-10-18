//! Subsonic vertical tail (VTPLNF), ventral fin (VFPLNF) and twin vertical
//! panels (TVTPAN) without a body. Ports M08O10 (VTDRAG and
//! VFDRAG: zero-lift drag only, DATCOM 4.1.5.1), SUBLAT's tail section
//! (5.3.1.1 sideforce, yawing and rolling moment derivatives) and M17O21's
//! blanking of the alpha-dependent lateral slots.
//!
//! Host quirk: SUBLAT's tail section follows its host surface's part (the
//! wing in M17O21's first call, the HT in the second), and without a body
//! that part returns early. So the VT section only runs without a wing and
//! the VF section only without an HT; the host A, B and STB it reads are
//! then the absent surface's INITZE state (A all UNUSED, B(2) = beta).

use super::{
    data::{cdrag as t, sublat as s},
    planform_inputs, tb, tl, WingCase, STRA,
};
use crate::{
    aero::{
        body::{fig26, X27I, X27M},
        AeroError, Result,
    },
    flight::FlightPoint,
    geometry::{Geometry, SurfaceGeometry},
    input::{Namelist, UNUSED},
    math::{finite, powr},
    tables::{PI, RAD},
};

/// One fin's persistent state: AVT/AVF(1..195) after M02O02 and
/// VTIN/VFIN(1..154), 1-based like the Fortran.
struct Fin {
    a: [f32; 196],
    ain: [f32; 155],
}

impl Fin {
    /// `offset` is the section namelist's value index of slot 1: VFSCHR
    /// starts at /VTI/ word 163, so VFIN(k) is its value 161 + k (0-based).
    fn new(
        surface: &SurfaceGeometry,
        planform: &Namelist,
        section: &Namelist,
        offset: usize,
    ) -> Self {
        let mut a = [0.0; 196];
        a[1..].copy_from_slice(&surface.coefficients[..195]);
        let mut ain = [UNUSED; 155];
        ain[1..16].copy_from_slice(&planform_inputs(&surface.planform));
        // VTIN/VFIN(7..8) hold the namelist's own sweeps (INPUT zeroes them);
        // only the geometry (AVF) uses XNAM20's VT sweep cache.
        ain[7] = planform.scalar("SAVSI").unwrap_or(0.0);
        ain[8] = planform.scalar("SAVSO").unwrap_or(0.0);
        // VTIN(10) is VTSCHR's ALPHAO (XNAM20 skips VFIN(10)); neither is
        // read by the subsonic fin methods.
        if offset == 0 {
            ain[10] = section.values[9];
        }
        ain[16..155].copy_from_slice(&section.values[offset + 15..offset + 154]);
        // VTIN(64) is the LOGICAL CAMV, which INPUT resets to .FALSE. (0).
        if offset == 0 {
            ain[64] = if ain[64] == 1.0 { 1.0 } else { 0.0 };
        }
        Fin { a, ain }
    }
}

/// Results for one fin at a flight point (VT or VF IOM blocks).
#[derive(Debug, Clone)]
pub struct FinAero {
    /// VT(1..NALPHA): zero-lift CD in the first slot, -UNUSED after it.
    pub cd: Vec<f32>,
    /// Lateral derivatives (per degree): CYbeta and CNbeta are only given
    /// in the first slot (-UNUSED after it), CLbeta per alpha.
    pub cyb: Vec<f32>,
    pub cnb: Vec<f32>,
    pub clb: Vec<f32>,
    pub work: FinWork,
}

/// 0-based copies of the fin's COMMON arrays after the point.
#[derive(Debug, Clone)]
pub struct FinWork {
    /// VT/VF(1..200).
    pub iom: Vec<f32>,
    /// AVT/AVF(1..195) and VTIN/VFIN(1..154); empty for twin panels alone.
    pub a: Vec<f32>,
    pub ain: Vec<f32>,
    /// DVT/DVF(1..55).
    pub d: Vec<f32>,
    /// The host's STB (VT) or STBH (VF) after SUBLAT, when its tail section
    /// ran (i.e. without the host surface).
    pub stb: Option<Vec<f32>>,
}

/// Per-case fin state plus the case constants the routines read.
pub struct Fins {
    vertical: Option<Fin>,
    ventral: Option<Fin>,
    /// TVTIN(1..8) when TVTPAN is given.
    twin: Option<[f32; 9]>,
    /// WGPL and HTPL: hosts present (their tail sections are skipped).
    wing: bool,
    horizontal_tail: bool,
    /// SYNA(1..19), 1-based.
    syna: [f32; 20],
    alphas: Vec<f32>,
    sref: f32,
    blref: f32,
    roughness: f32,
}

/// A fin's per-point arrays: the IOM block, D and the host STB.
struct Point {
    iom: [f32; 381],
    d: [f32; 56],
    stb: [f32; 136],
    section: bool,
}

impl Point {
    fn new() -> Self {
        Point {
            iom: [UNUSED; 381],
            d: [UNUSED; 56],
            stb: [UNUSED; 136],
            section: false,
        }
    }
}

impl Fins {
    /// `None` without VTPLNF, VFPLNF and TVTPAN.
    pub fn new(geometry: &Geometry) -> Result<Option<Self>> {
        let case = &geometry.inputs.case;
        let schema = |name: &str| case.namelist(name).expect("fin schemas");
        let vertical = geometry
            .vertical_tail
            .as_ref()
            .map(|v| Fin::new(v, schema("VTPLNF"), schema("VTSCHR"), 0));
        let ventral = geometry
            .ventral_fin
            .as_ref()
            .map(|v| Fin::new(v, schema("VFPLNF"), schema("VFSCHR"), 162));
        let tvtpan = schema("TVTPAN");
        let twin = tvtpan.present.then(|| {
            let mut t = [UNUSED; 9];
            t[1..].copy_from_slice(&tvtpan.values[154..162]);
            t
        });
        if vertical.is_none() && ventral.is_none() && twin.is_none() {
            return Ok(None);
        }
        let mut syna = [UNUSED; 20];
        syna[1..].copy_from_slice(&schema("SYNTHS").values[..19]);
        let fl = schema("FLTCON");
        let nalpha = ((fl.values[1] + 0.5) as i32).clamp(0, 20) as usize;
        let reference = &geometry.reference;
        Ok(Some(Fins {
            vertical,
            ventral,
            twin,
            wing: geometry.wing.is_some(),
            horizontal_tail: geometry.horizontal_tail.is_some(),
            syna,
            alphas: fl.values[22..22 + nalpha].to_vec(),
            sref: reference.area.ok_or_else(|| {
                AeroError::Invalid("SREF is required for tail aerodynamics".into())
            })?,
            blref: reference.span.unwrap_or(2.0 * UNUSED),
            roughness: reference.roughness,
        }))
    }

    /// MAIN01's fin sequence at a subsonic point: INITZ1, M08O10, EXSUBT's
    /// CDV, then M17O21's SUBLAT tail sections and blanking. `ht` is the
    /// horizontal tail after its own MAIN01 overlays (SUBLAT's endplate
    /// term reads AHT and HTIN). Returns (vertical tail, ventral fin).
    pub fn subsonic(
        &mut self,
        point: &FlightPoint,
        experimental: Option<&Namelist>,
        ht: Option<&WingCase>,
    ) -> Result<(Option<FinAero>, Option<FinAero>)> {
        let i = point.mach_index + 1;
        let (mach, beta) = (point.mach, (1.0 - point.mach * point.mach).abs().sqrt());
        // MAIN00: AVT/AVF(129) = Reynolds number per unit length; MAIN01:
        // (131)/(132) = this Mach's section CLalpha and CLmax.
        for fin in [&mut self.vertical, &mut self.ventral]
            .into_iter()
            .flatten()
        {
            fin.a[129] = point.reynolds;
            fin.a[131] = fin.ain[i + 20];
            fin.a[132] = fin.ain[i + 40];
        }
        let (mut vt, mut vf) = (Point::new(), Point::new());
        // M08O10: drag, then both IOM arrays regardless of which fin exists.
        if self.vertical.is_some() || self.ventral.is_some() {
            for (fin, p) in [(&self.vertical, &mut vt), (&self.ventral, &mut vf)] {
                if let Some(fin) = fin {
                    self.vtdrag(fin, &mut p.d, mach)?;
                }
                p.iom[1] = p.d[20];
                for k in [21, 41, 61, 101, 121] {
                    p.iom[k] = 0.0;
                }
                for j in 2..=self.alphas.len() {
                    for base in [0, 20, 40, 60, 80, 100, 120] {
                        p.iom[j + base] = -UNUSED;
                    }
                }
            }
        }
        // EXSUBT: XNAM23 maps EXPR's CDV to VT(1).
        if let Some(cdv) = experimental.map(|e| e.values[460]) {
            if cdv != UNUSED {
                vt.iom[1] = cdv;
            }
        }
        // M29O35: STB(2) = (WINGIN(4)-WINGIN(12))/WINGIN(4) = 0 and STB(3)
        // = 1 without a wing; STBH stays UNUSED without an HT.
        vt.stb[2] = 0.0;
        vt.stb[3] = 1.0;
        let first = self.vertical.is_some() || self.twin.is_some();
        if first && !self.wing {
            let twin = self.twin;
            self.section(&mut vt, 0, (i, beta), ht, twin)?;
        }
        if self.ventral.is_some() && !self.horizontal_tail {
            self.section(&mut vf, 1, (i, beta), None, None)?;
        }
        // M17O21: CYbeta and CNbeta only at the first alpha.
        for p in [&mut vt, &mut vf] {
            for j in 2..=self.alphas.len() {
                p.iom[j + 140] = -UNUSED;
                p.iom[j + 160] = -UNUSED;
            }
        }
        let vertical = (self.vertical.is_some() || self.twin.is_some())
            .then(|| self.result(self.vertical.as_ref(), &vt))
            .transpose()?;
        let ventral = self
            .ventral
            .is_some()
            .then(|| self.result(self.ventral.as_ref(), &vf))
            .transpose()?;
        Ok((vertical, ventral))
    }

    /// VTDRAG/VFDRAG: skin-friction drag D(20) from FIG 4.1.5.1-26/27/28B,
    /// per panel for non-straight-tapered fins. Unlike CDRAG there is no
    /// zero-sweep fix and no drag due to lift.
    fn vtdrag(&self, fin: &Fin, d: &mut [f32; 56], fm: f32) -> Result<()> {
        let (a, vin, sref) = (&fin.a, &fin.ain, self.sref);
        d[2] = 12.0 * a[16] / self.roughness;
        // FIG 4.1.5.1-27 cutoff Reynolds number.
        let cept = tb(&X27M, &X27I, fm, 0, 0)?;
        let cutoff = powr(d[2], 1.0482) * powr(10.0, cept);
        if vin[15] == f32::from(STRA) {
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
                &[0, 2, 0, 2],
            )?;
            let capl = if vin[18] >= 0.30 { 1.20 } else { 2.0 };
            d[20] =
                d[10] * (1. + capl * vin[16] + 100. * vin[16].powi(4)) * d[13] * (2. * a[3] / sref);
            return Ok(());
        }
        // Inboard (A(1)) and outboard (A(2)) panels.
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
            &[0, 0, 0, 0],
        )?;
        let capl = if vin[18] < 0.30 { 2.00 } else { 1.20 };
        d[21] = d[11] * (1. + capl * vin[16] + 100. * vin[16].powi(4)) * d[23] * (2. * a[1] / sref);
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
        let capl = if vin[66] < 0.30 { 2.00 } else { 1.20 };
        d[22] = d[12] * (1. + capl * vin[65] + 100. * vin[65].powi(4)) * d[24] * (2. * a[2] / sref);
        d[20] = d[21] + d[22];
        Ok(())
    }

    /// SUBLAT's tail section (labels 1260-1330) for the VT (`ity` 0, with
    /// the HT endplate and twin panels) or the VF (`ity` 1). The host A
    /// slots it reads (4, 43, 120) are UNUSED and STB(4)/STB(59) = 1.
    fn section(
        &self,
        p: &mut Point,
        ity: u8,
        (i, beta): (usize, f32),
        ht: Option<&WingCase>,
        twin: Option<[f32; 9]>,
    ) -> Result<()> {
        let (n, sref, blref, syna) = (self.alphas.len(), self.sref, self.blref, &self.syna);
        let (host_a4, host_a43, host_a120) = (UNUSED, UNUSED, UNUSED);
        let (iom, stb) = (&mut p.iom, &mut p.stb);
        p.section = true;
        iom[141] = 0.0;
        iom[161] = 0.0;
        for j in 1..=n {
            iom[j + 180] = 0.0;
        }
        let fin = if ity == 0 {
            self.vertical.as_ref()
        } else {
            self.ventral.as_ref()
        };
        if let Some(fin) = fin {
            let (avt, vin) = (&fin.a, &fin.ain);
            // FIG 5.3.1.1-22A/D: panel lift-slope factor and span ratio.
            // Without a body the fin span equals its exposed span and the
            // original's ARG2 is +Inf, which both lookups hold at the table
            // end (no extrapolation); f32::MAX does the same while passing
            // rudatcom's finite-input checks.
            let arg2 = (vin[4] / (2.0 * (vin[4] - vin[3]))).min(f32::MAX);
            stb[120] = tl(
                &[&s::X122A, &s::X222A],
                &s::Y22A,
                &[avt[118], arg2],
                &[0, 0, 0, 0],
            )?;
            stb[118] = tb(&s::X5322D, &s::Y5322D, arg2, 0, 0)?;
            // HT endplate (FIG 5.3.1.1-22B/C) when the HT is above the
            // fin root; a NaN ZH also takes this branch, as in Fortran.
            let endplate = ht.filter(|_| ity == 0).and_then(|h| {
                let zh = syna[7]
                    - ((h.ain[4] - h.ain[3]) * h.a[62] + h.a[30] - h.a[16] / 4.0)
                        * (syna[8] / RAD).sin();
                (zh >= 0.0 || zh.is_nan()).then_some((h, zh))
            });
            if let Some((h, zh)) = endplate {
                let at = [stb[9] / stb[10], -zh / vin[4]];
                stb[121] = tl(&[&s::X122B, &s::X222B], &s::Y22B, &at, &[0, 0, 2, 0])?;
                stb[119] = tb(&s::X5322C, &s::Y5322C, (h.a[2] + h.a[119]) / avt[4], 0, 0)?;
            } else {
                stb[121] = 0.0;
                stb[119] = 0.0;
            }
            stb[116] = stb[120] * avt[120] * (1. + stb[119] * (stb[121] - 1.));
            let bpart = 3.06 * ((avt[4] / host_a4) / (1. + host_a43));
            let cpart = 0.4 * stb[4] / stb[59] + 0.009 * host_a120;
            stb[117] = 0.724 + bpart + cpart;
            // Effective aspect-ratio lift slope with the section CLalpha.
            let anum = 2. * PI * stb[116];
            let k = vin[i + 20] * RAD / (2. * PI);
            let apart = stb[116] * stb[116] / (k * k);
            let bpart = 1. + avt[50] * avt[50] / (beta * beta);
            stb[5] = anum / (2. + (apart * bpart + 4.).sqrt());
            iom[141] = -stb[118] * stb[5] * stb[117] * avt[4] / (RAD * sref);
            iom[161] = -iom[141] * stb[11] / blref;
            for (j, &alpha) in self.alphas.iter().enumerate() {
                let (sa, ca) = (alpha / RAD).sin_cos();
                iom[j + 181] = (iom[141] * (stb[12] * ca - stb[11] * sa)) / blref;
            }
            // Twin vertical tails (PHIV/PHIF given): 2 cos^2(dihedral).
            if !(syna[18] == UNUSED && syna[19] == UNUSED) {
                let phi = if ity == 0 { syna[18] } else { syna[19] };
                let c = (phi / RAD).cos();
                let tvcorr = 2. * (c * c);
                iom[141] *= tvcorr;
                iom[161] *= tvcorr;
                for j in 1..=n {
                    iom[j + 180] *= tvcorr;
                }
            }
        }
        let Some(tv) = twin else {
            return Ok(());
        };
        // FIG 5.3.1.1-24A/B/C: twin vertical panels; BD(1) (body length)
        // is UNUSED without a body.
        let bd1 = UNUSED;
        stb[75] = tb(&s::X5324A, &s::Y5324A, tv[1] / tv[2], 0, 0)?;
        stb[6] = tv[2] * tv[2] / tv[5];
        let arg2 = stb[75] * stb[6];
        stb[116] = arg2;
        stb[73] = tl(
            &[&s::X124B, &s::X224B],
            &s::Y24B,
            &[tv[6], arg2],
            &[0, 2, 0, 2],
        )?;
        let at = [tv[4] / bd1, tv[3] / tv[2]];
        stb[74] = tl(&[&s::X124C, &s::X224C], &s::Y24C, &at, &[2, 0, 0, 0])?;
        let tvt = -stb[74] * stb[73] * 2. * tv[5] / (sref * RAD);
        iom[141] += tvt;
        iom[161] -= tvt * tv[7] / blref;
        for (j, &alpha) in self.alphas.iter().enumerate() {
            let (sa, ca) = (alpha / RAD).sin_cos();
            iom[j + 181] += tvt * (tv[8] * ca - tv[7] * sa) / blref;
        }
        Ok(())
    }

    fn result(&self, fin: Option<&Fin>, p: &Point) -> Result<FinAero> {
        let n = self.alphas.len();
        let take = |base: usize| -> Result<Vec<f32>> {
            let v = p.iom[base..base + n].to_vec();
            finite(&v)?;
            Ok(v)
        };
        Ok(FinAero {
            cd: take(1)?,
            cyb: take(141)?,
            cnb: take(161)?,
            clb: take(181)?,
            work: FinWork {
                iom: p.iom[1..201].to_vec(),
                a: fin.map_or(Vec::new(), |f| f.a[1..].to_vec()),
                ain: fin.map_or(Vec::new(), |f| f.ain[1..].to_vec()),
                d: p.d[1..].to_vec(),
                stb: p.section.then(|| p.stb[1..].to_vec()),
            },
        })
    }
}
