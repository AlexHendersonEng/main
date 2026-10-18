//! Supersonic wing alone (MAIN04): M18O22's SUPDRG (DATCOM 4.1.5.1/4.1.5.2
//! zero-lift and lift-dependent drag factors), M27O33's SUPLNG (4.1.3.2
//! normal-force slope, 4.1.3.3 nonlinear lift with ANGDET/FIG60B shock
//! detachment, 4.1.4.2 aerodynamic centre) plus its CLalpha pass, and
//! M23O27's SUPLAT (5.1.1.1/7.1.1.1/7.1.2.2 lateral derivatives).
//!
//! The routines share COMMON /SUPWH/ SLG(141), reset to UNUSED per point by
//! INITZ1; `slg` holds it 1-based and the `G_*` constants name its slots.
//! TANLEI/TANLEO = 0 are bumped to 1e-5 while SUPDRG/SUPLNG run, as the
//! original does to avoid dividing by zero.

use super::{
    data::{fig60b as f60, supdrg as d, suplat as l, suplng as t},
    lim,
    moment::fwdxac,
    tb, tl, WingCase, STRA,
};
use crate::{
    aero::{
        body::{fig26, X27I, X27M},
        AeroError, Result,
    },
    input::UNUSED,
    interp::{self, Limits},
    math::powr,
    tables::{PI, RAD},
};

// SLG slots (SUPDRG/SUPLNG EQUIVALENCE names).
const G_BETA: usize = 1;
const G_BOVERT: usize = 2;
const G_CNNNT: usize = 3;
const G_BCNA: usize = 4;
const G_CNTHRY: usize = 5;
const G_CNA1: usize = 7;
const G_DELTYT: usize = 8;
const G_DELTDT: usize = 9;
const G_TLE192: usize = 10;
const G_E: usize = 11;
const G_CC: usize = 12;
const G_CNAAA: usize = 12; // CNAAA(J) = SLG(12+J)
const G_ALPHAJ: usize = 32; // ALPHAJ(J) = SLG(32+J), radians incl. incidence
const G_CDL: usize = 52; // CDL(J) = SLG(52+J)
const G_A2: usize = 73;
const G_S2: usize = 74;
const G_CNAAAP: usize = 75;
const G_XACCRI: usize = 76;
const G_CNTBW: usize = 77;
const G_XACCRO: usize = 78;
const G_CDW: usize = 79;
const G_CDO: usize = 80;
const G_DRAGC: usize = 81;
const G_P: usize = 82;
const G_CFO: usize = 83;
const G_CFI: usize = 84;
const G_RNO: usize = 85;
const G_RNI: usize = 86;
const G_CDF: usize = 87;
const G_CF: usize = 88;
const G_RLCOFF: usize = 89;
const G_RNN: usize = 90;
const G_CNAO: usize = 91;
const G_CNAI: usize = 92;
const G_RMACH: usize = 93;
const G_DETACH: usize = 94;
const G_DETANG: usize = 115;
const G_CNAAST: usize = 116;
const G_DETALP: usize = 117;
const G_CRBW: usize = 118;
const G_SBW: usize = 119;
const G_ARBW: usize = 120;
const G_TAPBW: usize = 121;
const G_CLEBW: usize = 122;
const G_CRGLV: usize = 123;
const G_SGLV: usize = 124;
const G_ARGLV: usize = 125;
const G_BE: usize = 126;
const G_CN1: usize = 127;
const G_CN2: usize = 128;
const G_CNAE: usize = 129;
const G_CNAGLV: usize = 130;
const G_CNABW: usize = 131;
const G_CLEGLV: usize = 132;
const G_RKL: usize = 133;
const G_XACCR: usize = 134;
const G_DCMCL: usize = 135;
const G_CMA: usize = 136;
const G_CNCNTI: usize = 137;
const G_CNCNTO: usize = 138;
const G_CNATI: usize = 139;
const G_CNATO: usize = 140;

/// INTERX on a TABLE(LIND,k) block: axis k is `table[k*lind..][..len[k]]`
/// and DEP is read with the strides of `len`, so a stale LENGTH (SUPLNG
/// reuses LGB(1) = 12 for the 56A-F lookup at label 1350) reads the table
/// exactly as the original does. `flags` are LX1L..LX4L, LX1U..LX4U.
pub(super) fn ix(
    table: &[f32],
    lind: usize,
    len: &[usize],
    dep: &[f32],
    at: &[f32],
    flags: [i32; 8],
) -> Result<f32> {
    let axes: Vec<&[f32]> = len
        .iter()
        .enumerate()
        .map(|(k, &n)| &table[k * lind..k * lind + n])
        .collect();
    let limits: Vec<Limits> = (0..len.len())
        .map(|k| Limits {
            lower: lim(flags[k]),
            upper: lim(flags[4 + k]),
        })
        .collect();
    let count: usize = len.iter().product();
    Ok(interp::interx(&axes, &dep[..count], at, &limits)?.value)
}

/// ARCSIN: the original's arcsine via ATAN (1000 outside [-1, 1]).
fn arcsin(a: f32) -> f32 {
    if a.abs() == 1.0 {
        PI / 2.0 * a / a.abs()
    } else if a.abs() > 1.0 {
        1000.0
    } else {
        (a / (1.0 - a.powi(2)).sqrt()).atan()
    }
}

/// ANGDET: maximum wedge deflection (radians) for an attached shock at
/// `mach` (NACA TR 1135 eqns 168 and 138, gamma = 1.4).
fn angdet(mach: f32) -> f32 {
    let gam: f32 = 1.4;
    let m2 = mach.powi(2);
    let sin2tm = ((1. + gam) * m2 - 4.
        + ((gam + 1.) * ((gam + 1.) * mach.powi(4) + 8. * (gam - 1.) * m2 + 16.)).sqrt())
        / (4. * gam * m2);
    let tdm = arcsin(sin2tm.sqrt());
    let cotd = tdm.tan() * (((gam + 1.) * m2) / (2. * (m2 * tdm.sin().powi(2) - 1.)) - 1.);
    (1. / cotd).atan()
}

/// FIG60B: CN per sin^2(alpha) for a supersonic leading edge with attached
/// shock. Builds beta*tan(alpha) at each tabulated CN for this beta, then
/// inverts that curve at `btana` (held at its last point).
fn fig60b(beta: f32, btana: f32) -> Result<f32> {
    let mut bta = [0.0f32; 13];
    for (b, &x) in bta.iter_mut().zip(&f60::XA).skip(1) {
        *b = tl(&[&f60::YA, &f60::XA], &f60::Z60B, &[beta, x], &[1, 1, 0, 0])?;
    }
    let btan = btana.min(bta[12]);
    tb(&bta, &f60::XA, btan, 0, 0)
}

impl WingCase {
    /// MAIN04's wing sequence for one supersonic point (MAIN00 has set B(1),
    /// B(2) and A(129)). WING, C, D, STB and SLG start as UNUSED (INITZ1).
    pub fn supersonic(
        &mut self,
        point: &crate::flight::FlightPoint,
        experimental: Option<&crate::input::Namelist>,
    ) -> Result<super::WingAero> {
        self.main00(point);
        self.main04(experimental)?;
        self.result()
    }

    /// MAIN04's wing overlays at `self.mach` with B(1)/B(2) left as they are
    /// (MAIN00's, or the transonic pre-pass's, which no routine here reads).
    pub(super) fn main04(&mut self, experimental: Option<&crate::input::Namelist>) -> Result<()> {
        self.initz1();
        self.exsubt(experimental);
        self.m18o22_restore();
        self.supdrg()?;
        self.suplng()?;
        self.m27o33_slopes()?;
        self.exsubt(experimental);
        self.suplat()?;
        // M23O27: only the first-alpha lateral derivatives are reported.
        for j in 2..=self.alphas.len() {
            self.wing[j + 140] = -UNUSED;
            self.wing[j + 160] = -UNUSED;
        }
        Ok(())
    }

    /// M18O22 reruns WTGEOM: slots it fills are restored, others (written
    /// by earlier points) persist. A(147..158) are INITZE angle caches that
    /// only LIFTCF rewrites.
    pub(super) fn m18o22_restore(&mut self) {
        for k in (1..196).filter(|k| !(147..=158).contains(k)) {
            if self.a_geometry[k] != UNUSED {
                self.a[k] = self.a_geometry[k];
            }
        }
    }

    /// TANLEO/TANLEI = 0 become 1e-5 inside SUPDRG/SUPLNG.
    fn bump_tangents(&mut self) {
        for k in [86, 62] {
            if self.a[k] == 0.0 {
                self.a[k] = 0.00001;
            }
        }
    }

    fn restore_tangents(&mut self) {
        for k in [86, 62] {
            if self.a[k] == 0.00001 {
                self.a[k] = 0.0;
            }
        }
    }

    /// ROUGFC as M18O22 leaves it (1.6e-4 when below 1e-10).
    pub(super) fn rough(&self) -> f32 {
        if self.roughness < 1.0e-10 {
            1.6e-4
        } else {
            self.roughness
        }
    }

    /// SUPDRG: skin friction (FIG 4.1.5.1-26/27, per panel for non-straight
    /// wings), wave drag (sharp or round leading edge) and, for straight
    /// tapered wings, the FIG 4.1.5.2-58 drag-due-to-lift factor DRAGC.
    fn supdrg(&mut self) -> Result<()> {
        self.bump_tangents();
        let (a, ain) = (self.a, self.ain);
        let mach = self.mach;
        let sr = self.sref;
        let beta = (mach.powi(2) - 1.).sqrt();
        self.slg[G_BETA] = beta;
        let straight = self.kind == STRA;
        let ruff = self.rough();
        // FIG 4.1.5.1-26 with the 4.1.5.1-27 cutoff Reynolds number.
        let skin = |cbar: f32, g: &mut [f32; 142]| -> Result<f32> {
            let mut rnn = cbar * a[129];
            let rach = mach.min(3.0);
            let arg = 12. * cbar / ruff;
            let cept = tb(&X27M, &X27I, rach, 0, 0)?;
            let rlcoff = powr(arg, 1.0482) * powr(10.0, cept);
            g[G_RLCOFF] = rlcoff;
            if rlcoff < rnn {
                rnn = rlcoff;
            }
            g[G_RNN] = rnn;
            let cf = fig26(rnn, rach);
            g[G_CF] = cf;
            Ok(cf)
        };
        let cdf = if straight {
            let cf = skin(a[15], &mut self.slg)?;
            cf * a[3] / sr * 2.
        } else {
            // Inner panel at CBARI, outer at CBARO; equal chords skip the
            // inner store, leaving CFI UNUSED as in the original.
            let mut cf = skin(a[15], &mut self.slg)?;
            if a[15] != a[17] {
                self.slg[G_RNI] = self.slg[G_RNN];
                self.slg[G_CFI] = cf;
                cf = skin(a[17], &mut self.slg)?;
            }
            self.slg[G_RNO] = self.slg[G_RNN];
            self.slg[G_CFO] = cf;
            (self.slg[G_CFI] * a[1] + cf * a[2]) / sr * 2.
        };
        self.slg[G_CDF] = cdf;
        let a = self.a;
        // Wave drag on the basic wing (SBW is SUPLNG's, still UNUSED here).
        let (ta, s, ca, lerbw) = if straight {
            (a[62], a[3], a[61], ain[62] * ((ain[6] + ain[1]) / 2.))
        } else {
            (
                a[86],
                self.slg[G_SBW],
                a[85],
                ain[63] * ((ain[6] + ain[1] + 2. * ain[5]) / 4.),
            )
        };
        let bovert = beta / ta;
        self.slg[G_BOVERT] = bovert;
        let (ksharp, tceff, spans) = (ain[71], ain[70], ain[3]);
        let cdw = if ksharp != UNUSED {
            let arg = ksharp * tceff.powi(2) * s / sr;
            if bovert < 1. {
                arg / ta
            } else {
                arg / beta
            }
        } else {
            let m3 = mach.powi(3);
            let arg1 = 1.28 * m3 * ca.powi(6) / (1. + m3 * ca.powi(3));
            let arg2 = 2. * lerbw * (2. * spans) / (sr * ca);
            let cdle = arg1 * arg2;
            let arg = 16. * tceff.powi(2) * s / (3. * sr);
            if bovert >= 1. {
                cdle + arg / beta
            } else {
                cdle + arg / ta
            }
        };
        self.slg[G_CDW] = cdw;
        self.slg[G_CDO] = cdf + cdw;
        // Non-straight wings return here with the tangents still bumped.
        if !straight {
            return Ok(());
        }
        let rlw = ain[1] + a[18] * a[10];
        let p = a[3] / (rlw * 2. * spans);
        self.slg[G_P] = p;
        let arg1 = beta * spans / rlw;
        let dep: &[f32] = if ksharp == UNUSED {
            &d::DR5258
        } else {
            &d::DS5258
        };
        self.slg[G_DRAGC] = ix(&d::T15258, 5, &[5], dep, &[arg1], [0, 0, 0, 0, 1, 0, 0, 0])?;
        self.restore_tangents();
        Ok(())
    }

    /// SUPLNG labels 1030/1040: (CNalpha/CNalpha theory) from FIG 4.1.3.2-60A
    /// (subsonic edge, at BOVERT) or 60B (supersonic, at 1/BOVERT and the
    /// thickness angle DELTDT), capped at 1. Returns CNCNT and VAB(1..2).
    fn cncnt(&mut self, bovert: f32, ca: f32, suple: bool) -> Result<(f32, [f32; 2])> {
        let deltay = self.ain[17];
        let deltyt = deltay / ca;
        self.slg[G_DELTYT] = deltyt;
        let (cncnt, vab) = if !suple {
            let vab = [bovert, deltyt];
            let flags = [0, 0, 0, 0, 0, 2, 0, 0];
            (ix(&t::A1350, 9, &[9, 8], &t::DA50, &vab, flags)?, vab)
        } else {
            let deltdt = (deltay / (5.85 * ca)).atan() * RAD;
            self.slg[G_DELTDT] = deltdt;
            let vab = [1. / bovert, deltdt];
            let flags = [0, 0, 0, 0, 0, 2, 0, 0];
            (ix(&t::B1350, 11, &[11, 8], &t::DB50, &vab, flags)?, vab)
        };
        Ok((cncnt.min(1.), vab))
    }

    /// SUPLNG label 1320: FIG 4.1.3.2-56A-F theoretical slope, BCNA over
    /// the edge tangent (subsonic edge) or over beta (supersonic edge).
    fn cnthry(&mut self, vab: [f32; 3], ta: f32, suple: bool) -> Result<f32> {
        let flags = [0, 2, 0, 0, 0, 2, 0, 0];
        let bcna = ix(&t::T13246, 23, &[23, 8, 6], &t::D13246, &vab, flags)?;
        self.slg[G_BCNA] = bcna;
        let beta = self.slg[G_BETA];
        let cnthry = if suple { bcna / beta } else { bcna / ta };
        self.slg[G_CNTHRY] = cnthry;
        Ok(cnthry)
    }

    /// SUPLNG labels 1430-1455: FIG 4.1.4.2-26A-F aerodynamic centre
    /// XAC/CR* (FWDXAC for forward sweep, input XAC for unswept edges).
    fn xaccr(&mut self, bovert: f32, vab: [f32; 2]) -> Result<f32> {
        let flags = [0, 2, 0, 0, 0, 2, 0, 0];
        let mut xaccr = if bovert > 1. {
            ix(
                &t::T422AF,
                6,
                &[6, 6, 6],
                &t::SUPAF,
                &[1. / bovert, vab[0], vab[1]],
                flags,
            )?
        } else {
            ix(
                &t::T422AF,
                6,
                &[6, 6, 6],
                &t::SUBAF,
                &[bovert, vab[0], vab[1]],
                flags,
            )?
        };
        if bovert < 0. {
            xaccr = fwdxac(vab[0], vab[1], 1. / bovert, self.mach)?;
        }
        let input = self.ain[self.i + 71];
        if self.a[58] < 1.0e-10 && input != UNUSED {
            xaccr = input * self.a[16] / self.a[10];
        }
        self.slg[G_XACCR] = xaccr;
        Ok(xaccr)
    }

    /// SUPLNG: normal-force slope CNA1, the CL/CN curve (straight tapered
    /// wings only), pitching moment slope and the CD/CN/CA polars.
    fn suplng(&mut self) -> Result<()> {
        self.bump_tangents();
        let a = self.a;
        let ain = self.ain;
        let mach = self.mach;
        let sw = self.sref;
        let n = self.alphas.len();
        let beta = (mach.powi(2) - 1.).sqrt();
        self.slg[G_BETA] = beta;
        let (xcg, xw, aliw) = self.syna;
        for j in 1..=n {
            self.slg[G_ALPHAJ + j] = (self.alphas[j - 1] + aliw) / RAD;
        }
        let alphaj = |s: &Self, j: usize| s.slg[G_ALPHAJ + j];
        let (tanle, cosle, ar, srstar) = (a[62], a[61], a[7], a[3]);
        let (tanlei, tanleo) = (a[62], a[86]);
        let (spans, spano, span) = (ain[3], ain[2], ain[4]);
        let straight = self.kind == STRA;
        if straight {
            let bovert = beta / tanle;
            self.slg[G_BOVERT] = bovert;
            let suple = bovert > 1.0;
            let (cncnt, _) = self.cncnt(bovert, cosle, suple)?;
            self.slg[G_CNNNT] = cncnt;
            // 1050: rectangular wings use FIG 56G or the closed form.
            let cnthry = if a[25] == 1.0 && a[58] == 0.0 {
                if ar * beta > 1.0 {
                    let bcna = 4. - 2. * (1. / (ar * beta));
                    self.slg[G_BCNA] = bcna;
                    bcna / beta
                } else {
                    let flags = [0; 8];
                    ix(&t::G13246, 9, &[9], &t::DG3246, &[ar * beta], flags)? * ar
                }
            } else {
                self.cnthry([bovert, ar * tanle, a[27]], tanle, suple)?
            };
            self.slg[G_CNTHRY] = cnthry;
            let cna = cnthry * cncnt;
            self.slg[G_CNA1] = cna * srstar / sw;
            let tle192 = tanle / 1.92;
            self.slg[G_TLE192] = tle192;
            // FIG 4.1.3.3-59A: E for highly swept edges.
            let e = if tle192 > 1. {
                let deltdt = (ain[17] / (5.85 * cosle)).atan() * RAD;
                self.slg[G_DELTDT] = deltdt;
                let flags = [0, 2, 0, 0, 2, 2, 0, 0];
                let vab = [deltdt, cna * tle192];
                let cc = ix(&t::A13359, 11, &[9, 11], &t::D13359, &vab, flags)?;
                self.slg[G_CC] = cc;
                cna * (tle192 + cc * (tle192 - 1.))
            } else {
                cna
            };
            self.slg[G_E] = e;
            // CN from the linear term plus CNAAA sin(a)|sin(a)|.
            let polar = move |s: &mut Self, j: usize, cnaaa: f32| {
                let al = alphaj(s, j);
                s.slg[G_CNAAA + j] = cnaaa;
                let cn =
                    (cna * (2. * al).sin() / 2. + cnaaa * al.sin() * al.sin().abs()) * srstar / sw;
                s.wing[60 + j] = cn;
                s.wing[20 + j] = cn * al.cos();
            };
            if !suple {
                // FIG 4.1.3.3-59B, right or left side.
                let arg = e * bovert;
                let flags = [2, 2, 0, 0, 0, 0, 0, 0];
                for j in 1..=n {
                    let al = alphaj(self, j);
                    let cnaaa = if al == 0.0 {
                        0.0
                    } else {
                        let arg1 = cncnt / (beta * al.tan().abs());
                        if arg1 > 1. {
                            ix(
                                &t::BL3359,
                                10,
                                &[5, 10],
                                &t::DL3359,
                                &[1. / arg1, arg],
                                flags,
                            )?
                        } else {
                            ix(&t::BR3359, 11, &[11, 10], &t::DR3359, &[arg1, arg], flags)?
                        }
                    };
                    polar(self, j, cnaaa);
                }
            } else {
                self.supersonic_edge(cncnt, e, &polar)?;
            }
        } else {
            // 1310: basic wing (leading edge outboard sweep to the tip).
            let (tanteo, cb, ct) = (a[104], ain[5], ain[1]);
            let spanin = spans - spano;
            let crbw = cb + spanin * (tanleo - tanteo);
            let sbw = (crbw + ct) * spans;
            let arbw = 4. * spans.powi(2) / sbw;
            let tapbw = ct / crbw;
            self.slg[G_CRBW] = crbw;
            self.slg[G_SBW] = sbw;
            self.slg[G_ARBW] = arbw;
            self.slg[G_TAPBW] = tapbw;
            let bovert = beta / tanleo;
            self.slg[G_BOVERT] = bovert;
            let suple = bovert > 1.;
            let cnthry = self.cnthry([bovert, arbw * tanleo, tapbw], tanleo, suple)?;
            let cle = self.cle(bovert)?;
            self.slg[G_CNTBW] = cnthry;
            self.slg[G_CLEBW] = cle;
            let cnabw = if tanleo >= tanlei {
                // 1350: subtract the inboard triangle; LGB(1) is still 12
                // from the CLE lookup, so the 56A-F table is read with
                // that stride.
                let s2 = spanin.powi(2) * tanleo;
                let a2 = 4. * spanin.powi(2) / s2;
                self.slg[G_S2] = s2;
                self.slg[G_A2] = a2;
                let flags = [0, 2, 0, 0, 0, 2, 0, 0];
                let vab = [bovert, a2 * tanleo, 0.0];
                let bcna = ix(&t::T13246, 23, &[12, 8, 6], &t::D13246, &vab, flags)?;
                self.slg[G_BCNA] = bcna;
                let cnt2 = if bovert > 1. {
                    bcna / tanleo
                } else {
                    bcna / beta
                };
                (cnthry * sbw / sw - cnt2 * s2 / sw) * cle
            } else {
                cnthry * sbw / sw * cle
            };
            self.slg[G_CNABW] = cnabw;
            // Glove component when the leading-edge sweeps differ by >= 4 deg.
            let cnaglv = if (a[58] - a[82]).abs() < 4. {
                0.0
            } else {
                let crglv = tanlei * spanin;
                let sglv = crglv * spanin;
                let arglv = 4. * spanin.powi(2) / sglv;
                self.slg[G_CRGLV] = crglv;
                self.slg[G_SGLV] = sglv;
                self.slg[G_ARGLV] = arglv;
                let bovert = beta / tanlei;
                self.slg[G_BOVERT] = bovert;
                let suple = bovert > 1.;
                let cnthry = self.cnthry([bovert, arglv * tanlei, 0.0], tanlei, suple)?;
                let cleglv = self.cle(bovert)?;
                self.slg[G_CLEGLV] = cleglv;
                cnthry * cleglv * sglv / sw
            };
            self.slg[G_CNAGLV] = cnaglv;
            // Extension component (FIG 4.1.3.2-63) when trailing edges differ.
            let cnae = if (a[76] - a[100]).abs() < 4. {
                0.0
            } else {
                let be = 2. * spanin;
                self.slg[G_BE] = be;
                let flags = [2, 2, 0, 0, 2, 2, 0, 0];
                let lens = [14, 10];
                let at = [beta / tanlei, a[80] / tanlei];
                let cn1 = ix(&t::T13253, 14, &lens, &t::D13253, &at, flags)?;
                let at = [beta / tanlei, tanteo / tanlei];
                let cn2 = ix(&t::T13253, 14, &lens, &t::D13253, &at, flags)?;
                self.slg[G_CN1] = cn1;
                self.slg[G_CN2] = cn2;
                (cn1 - cn2) * be.powi(2) / sw
            };
            self.slg[G_CNAE] = cnae;
            // FIG 4.1.3.2-61 interference factor KL (CLEGLV is UNUSED
            // without a glove, making the argument 0).
            let vab = [cnaglv / (self.slg[G_CLEGLV] * beta) * sw / a[4]];
            let rkl = ix(
                &t::T13251,
                12,
                &[12],
                &t::D13251,
                &vab,
                [2, 0, 0, 0, 2, 0, 0, 0],
            )?;
            self.slg[G_RKL] = rkl;
            self.slg[G_CNA1] = rkl * (cnabw + cnaglv + cnae);
        }
        // 1420: pitching moment slope about the moment reference.
        let dxw = (span - spans) * tanlei * (aliw / RAD).cos();
        let detxcg = xcg - (xw + dxw);
        self.a[173] = detxcg;
        let crstar = a[10];
        let xaccr = if straight {
            let bovert = self.slg[G_BOVERT];
            self.xaccr(bovert, [a[7] * tanle, a[27]])?
        } else {
            self.panel_xac()?
        };
        let cna1 = self.slg[G_CNA1];
        let dcmcl = (detxcg / crstar - xaccr) * crstar / self.cbarr;
        let cma = dcmcl * cna1;
        self.slg[G_DCMCL] = dcmcl;
        self.slg[G_CMA] = cma;
        for j in 1..=n {
            let al = alphaj(self, j);
            if al <= 0.175 {
                self.wing[40 + j] = cma * al;
            }
        }
        // 1540: drag due to lift and the body-axis forces.
        let p = self.slg[G_P];
        let arg = (1. + p) / (PI * ar * p);
        let (dragc, cdo) = (self.slg[G_DRAGC], self.slg[G_CDO]);
        for j in 1..=n {
            let cl = self.wing[20 + j];
            let cdl = dragc * arg * sw / srstar * cl.powi(2);
            self.slg[G_CDL + j] = cdl;
            let cd = cdo + cdl;
            self.wing[j] = cd;
            let alpha = self.alphas[j - 1] / RAD;
            let (cosa, sina) = (alpha.cos(), alpha.sin());
            if cl != UNUSED {
                self.wing[60 + j] = cl * cosa + cd * sina;
                self.wing[80 + j] = cd * cosa - cl * sina;
            }
        }
        if cma != UNUSED {
            self.wing[121] = cma / RAD;
        }
        if cna1 != UNUSED {
            self.wing[101] = cna1 / RAD;
        }
        self.restore_tangents();
        Ok(())
    }

    /// FIG 4.1.3.2-62 leading-edge factor (sharp edge when KSHARP is given).
    fn cle(&self, bovert: f32) -> Result<f32> {
        let (x, y): (&[f32], &[f32]) = if self.ain[71] == UNUSED {
            (&t::R13252, &t::DRND52)
        } else {
            (&t::S13252, &t::DSHP52)
        };
        ix(x, 12, &[12], y, &[bovert], [0; 8])
    }

    /// SUPLNG 1160-1300: nonlinear CNAAA for a supersonic leading edge.
    /// With an attached shock at zero alpha, the alpha where it detaches
    /// (DETANG, scanned in 1-degree steps up to the largest |alpha| in
    /// radians, as the original compares) splits FIG 60B (attached) from
    /// FIG 60A, blended linearly up to ALPHAP = DETANG + FIG 61A's delta.
    fn supersonic_edge(
        &mut self,
        cncnt: f32,
        e: f32,
        polar: &dyn Fn(&mut Self, usize, f32),
    ) -> Result<()> {
        let n = self.alphas.len();
        let beta = self.slg[G_BETA];
        let mach = self.mach;
        let (sinle, cosle) = (self.a[60], self.a[61]);
        let alphaj = |s: &Self, j: usize| s.slg[G_ALPHAJ + j];
        // FIG 4.1.3.3-60A, right (CNCNT/arg2 <= 1) or left side.
        let fig60a = |arg2: f32| -> Result<f32> {
            let arg1 = cncnt / arg2;
            if arg1 > 1. {
                ix(
                    &t::TL360A,
                    12,
                    &[6, 12],
                    &t::DL360A,
                    &[1. / arg1, e],
                    [0; 8],
                )
            } else {
                let flags = [2, 0, 0, 0, 0, 0, 0, 0];
                ix(&t::TR360A, 13, &[13, 12], &t::DR360A, &[arg1, e], flags)
            }
        };
        let rmach = mach * (1. - sinle.powi(2)).sqrt();
        self.slg[G_RMACH] = rmach;
        let deltdt = self.slg[G_DELTDT];
        let attached = rmach >= 1. && deltdt / RAD <= angdet(rmach);
        // DETACH is a LOGICAL in a REAL slot: .TRUE. is the integer 1.
        self.slg[G_DETACH] = f32::from_bits(u32::from(!attached));
        if !attached {
            for j in 1..=n {
                let al = alphaj(self, j);
                let cnaaa = if al == 0.0 {
                    0.0
                } else {
                    fig60a(beta * al.tan().abs())?
                };
                polar(self, j, cnaaa);
            }
            return Ok(());
        }
        let angmax = alphaj(self, 1).abs().max(alphaj(self, n).abs());
        let mut angle: f32 = 1.;
        let mut detang = None;
        // The first angle is always tried; ANGLE > ANGMAX is tested after.
        loop {
            let deg = angle / RAD;
            let rrmach = mach * powr(1. - (sinle * deg.cos()).powi(2), 0.5);
            if rrmach >= 1. && deltdt / RAD + deg.tan() / cosle >= angdet(rrmach) {
                detang = Some(deg);
                break;
            }
            angle += 1.;
            if angle > angmax {
                break;
            }
        }
        // ALPHAP and CNAAAP only exist once a detachment angle was found.
        let mut blend = None;
        let detang = match detang {
            Some(detang) => {
                let cnaast = fig60b(beta, beta * detang.tan())?;
                let flags = [0, 0, 0, 0, 2, 0, 0, 0];
                let delcn = ix(&t::T61A, 9, &[9], &t::SLOPE, &[beta], flags)?;
                let detalp = delcn * cnaast / RAD;
                let alphap = detang + detalp;
                let cnaaap = fig60a(beta * alphap.tan().abs())?;
                // CNAAA(1) briefly holds the ALPHAP value (overwritten below).
                self.slg[G_CNAAA + 1] = cnaaap;
                self.slg[G_CNAAST] = cnaast;
                self.slg[G_DETALP] = detalp;
                self.slg[G_CNAAAP] = cnaaap;
                blend = Some((alphap, cnaast, cnaaap));
                detang
            }
            None => 90. / RAD,
        };
        self.slg[G_DETANG] = detang;
        for j in 1..=n {
            let al = alphaj(self, j);
            let cnaaa = if al.abs() < detang {
                fig60b(beta, beta * al.tan().abs())?
            } else {
                let Some((alphap, cnaast, cnaaap)) = blend else {
                    return Err(AeroError::Invalid(
                        "SUPLNG: |alpha| >= 90 deg reads the original's unset ALPHAP".into(),
                    ));
                };
                if al.abs() < alphap {
                    let ratio = (al.abs() - detang) / (alphap - detang);
                    cnaast - ratio * (cnaast - cnaaap)
                } else if al == 0.0 {
                    0.0
                } else {
                    fig60a(beta * al.tan().abs())?
                }
            };
            polar(self, j, cnaaa);
        }
        Ok(())
    }

    /// SUPLNG 1500-1530: non-straight wings take XAC/CR* as the CNA-and-
    /// area-weighted mean of the inboard and outboard panels, each with its
    /// own CNCNT (FIG 60A/B) and 56A-F slope. VAB(1) for the slope lookup
    /// is whatever the CNCNT lookup left there (1/BOVERT on a supersonic
    /// edge), as in the original.
    fn panel_xac(&mut self) -> Result<f32> {
        let a = self.a;
        let beta = self.slg[G_BETA];
        let (crstar, tanleo, tanlei) = (a[10], a[86], a[62]);
        // (ARP, CA, TA, TAPR) for panels I = 1, 2.
        let panels = [(a[5], a[61], a[62], a[26]), (a[168], a[85], a[86], a[169])];
        let mut cna = [0.0f32; 2];
        for (i, &(arp, ca, ta, tapr)) in panels.iter().enumerate() {
            let bovert = beta / ta;
            self.slg[G_BOVERT] = bovert;
            let suple = bovert > 1.;
            let xaccr = self.xaccr(bovert, [arp * ta, tapr])?;
            if i == 0 {
                self.slg[G_XACCRI] = xaccr;
            } else {
                let arg1 = xaccr * a[166] / crstar;
                let arg2 = a[23] / (crstar * 2.) * tanleo;
                let arg3 = a[23] * tanlei / crstar;
                self.slg[G_XACCRO] = arg1 - arg2 + arg3;
            }
            let (cncnt, vab) = self.cncnt(bovert, ca, suple)?;
            let cnthry = self.cnthry([vab[0], arp * ta, tapr], ta, suple)?;
            let (gc, gt, gn) = if i == 0 {
                (G_CNCNTI, G_CNATI, G_CNAI)
            } else {
                (G_CNCNTO, G_CNATO, G_CNAO)
            };
            self.slg[gc] = cncnt;
            self.slg[gt] = cnthry;
            cna[i] = cncnt * cnthry;
            self.slg[gn] = cna[i];
        }
        let (si, so) = (a[1], a[167]);
        let (xi, xo) = (self.slg[G_XACCRI], self.slg[G_XACCRO]);
        let xaccr = (cna[0] * si * xi + cna[1] * so * xo) / (cna[0] * si + cna[1] * so);
        self.slg[G_XACCR] = xaccr;
        Ok(xaccr)
    }

    /// M27O33 after SUPLNG: CLalpha per alpha from the CL curve's TBFUNX
    /// slope; CMalpha and, for non-straight wings, everything beyond the
    /// first alpha are blanked with -UNUSED (CA(1) takes CD(1)).
    fn m27o33_slopes(&mut self) -> Result<()> {
        let n = self.alphas.len();
        let straight = self.kind == STRA;
        let hold = Limits {
            lower: lim(0),
            upper: lim(0),
        };
        for j in 2..=n {
            let cl = self.wing[21..21 + n].to_vec();
            let at = self.alphas[j - 1];
            self.wing[j + 100] = interp::tbfunx_unordered(&self.alphas, &cl, at, hold)?.derivative;
            self.wing[j + 120] = -UNUSED;
            if !straight {
                for base in [0, 20, 40, 60, 80, 100, 180] {
                    self.wing[j + base] = -UNUSED;
                }
            }
        }
        if !straight {
            self.wing[81] = self.wing[1];
        }
        Ok(())
    }

    /// SUPLAT's wing-alone part: CYbeta/CNbeta for rectangular (closed form)
    /// or subsonic-edge triangular wings (FIG 5.1.1.1-6, 7.1.1.1-8), and
    /// CLbeta from FIG 7.1.2.2-25. Straight tapered wings only; the
    /// wing-body terms that follow need a body (later checkpoint).
    fn suplat(&mut self) -> Result<()> {
        let mach = self.mach;
        let beta = (mach.powi(2) - 1.).sqrt();
        self.stb[1] = mach;
        self.stb[2] = beta;
        let (rm, rb) = (mach.powi(2), beta.powi(2));
        for k in 12..=14 {
            if self.ain[k] == UNUSED {
                self.ain[k] = 0.0;
            }
        }
        let (a, ain) = (self.a, self.ain);
        let arg = ain[3] - ain[12];
        let diheq = (ain[13] * arg + ain[14] * ain[12]) / ain[3];
        self.stb[4] = diheq;
        let scale = 2. * ain[4] / self.blref;
        if self.kind != STRA {
            return Ok(());
        }
        let (swepe, tapr, ar, tanle, cosle) = (a[59], a[118], a[120], a[62], a[61]);
        let (xcg, xw, _) = self.syna;
        let (cr, span, crbar) = (ain[6], ain[4], self.cbarr);
        let alp = |s: &Self, j: usize| (s.alphas[j - 1] / RAD).powi(2);
        if swepe == 0. && tapr == 1. && ar * beta >= 1.0 {
            let x = xw + 0.5 * cr - xcg;
            self.stb[3] = x;
            let arg1 = 1. / (PI * ar.powi(2) * rb);
            let arg2 = 4. * rm / 3. + 8. * rm * x / crbar;
            let arg3 = PI * ar * (1. - rb) * (3. + rb) / (3. * beta.powi(3));
            for j in 1..=self.alphas.len() {
                let al = alp(self, j);
                self.wing[140 + j] = -(al * 8.0 * rm / (RAD * PI * rb * ar)) - 0.0001 * diheq.abs();
                self.wing[160 + j] = scale * al * arg1 * (arg2 - arg3) / RAD;
            }
        } else {
            let arg = if tanle != 0.0 {
                beta / tanle
            } else {
                beta / 0.00001
            };
            if tapr == 0.0 && arg < 0.998 {
                let qbc = ix(&l::X51116, 16, &[16], &l::Y51116, &[arg], [0; 8])?;
                let ebc = ix(&l::X71118, 8, &[8], &l::Y71118, &[arg], [0; 8])?;
                self.stb[5] = qbc;
                self.stb[6] = ebc;
                let arg1 = PI * ar * rm / (4. * RAD * qbc);
                let x = xw + 0.6666 * span * tanle - xcg;
                self.stb[3] = x;
                let arg2 = PI / 3. * (ebc + (ar.powi(2) / 16. + x / crbar) * rm / qbc) / RAD;
                for j in 1..=self.alphas.len() {
                    let al = alp(self, j);
                    self.wing[140 + j] = -(al * arg1) - 0.0001 * diheq.abs();
                    self.wing[160 + j] = scale * al * arg2;
                }
            }
        }
        // 1040: CLbeta (CLPCTY is SUPLNG's CNNNT, CNAW its CNA1).
        let var = [beta * ar, ar * a[74], tapr];
        let flags = [0, 0, 0, 0, 2, 2, 0, 0];
        let clptoa = ix(&l::X12225, 17, &[17, 7, 5], &l::Y12225, &var, flags)?;
        let clp = clptoa * ar * self.slg[G_CNNNT];
        let clbd = 2. * diheq * (1. + 2. * tapr) * clp / (powr(RAD, 2.) * (1. + 3. * tapr));
        self.stb[7] = clptoa;
        self.stb[8] = clp;
        self.stb[9] = clbd;
        let arg1 = (1. + tapr * (1. + swepe)) * (1. + swepe / 2.) * tanle / beta;
        let arg2 = rm * cosle.powi(2) / ar + powr((tanle / 4.).abs(), 1.3333);
        let cnaw = self.slg[G_CNA1];
        for j in 1..=self.alphas.len() {
            let cnw = self.wing[60 + j];
            self.wing[180 + j] = scale * (clbd - 0.061 * cnw * cnaw * arg1 * arg2 / RAD);
        }
        Ok(())
    }
}
