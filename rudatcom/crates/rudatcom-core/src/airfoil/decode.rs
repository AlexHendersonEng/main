//! Ports of DECODE (NACA designation parsing and chordwise grid) and AIRFOL
//! (dispatch to the NACA family coordinate generators).

use super::{AirfoilError, Result, Section, SlotView, NP};
use crate::input::Case;

impl Section {
    /// Set IUM(index) (1-based). DECODE's IUM(10) sits after LLL in /IBW/;
    /// index 0 aliases LLL (rewritten by the grid loop), so it is ignored.
    fn set_digit(&mut self, index: i32, value: i32) -> Result<()> {
        match index {
            i if i <= 0 => Ok(()),
            1..=10 => {
                self.digits[index as usize - 1] = value;
                Ok(())
            }
            _ => Err(AirfoilError::Invalid(format!(
                "NACA designation '{}' has too many digits",
                self.card_text()
            ))),
        }
    }

    fn digit(&self, index: i32) -> Result<i32> {
        match index {
            1..=10 => Ok(self.digits[index as usize - 1]),
            _ => Err(AirfoilError::Invalid(format!(
                "NACA designation '{}' cannot be decoded",
                self.card_text()
            ))),
        }
    }

    pub(super) fn card_text(&self) -> String {
        String::from_utf8_lossy(&self.card).trim_end().to_string()
    }

    /// DECODE: read the digit fields I..LLL from columns 10-80 and build the
    /// 0.001/0.01/0.05/0.02 chordwise grid into X. Returns NA, the family:
    /// 1 4-digit, 2 mod 4-digit, 3 5-digit, 4 mod 5-digit, 5 1-series,
    /// 6 6-series, 7 supersonic.
    pub(super) fn decode(&mut self) -> Result<i32> {
        let mut na = match self.col(8) {
            b'1' => 5,
            b'4' => 1,
            b'5' => 3,
            b'6' => 6,
            b'S' => 7,
            _ => {
                // NA would be undefined in the original.
                return Err(AirfoilError::Invalid(format!(
                    "unknown NACA airfoil type in '{}' (column 8 must be 1, 4, 5, 6 or S)",
                    self.card_text()
                )));
            }
        };
        self.digits = [0; 10];
        let mut icount: i32 = 0;
        match na {
            6 => {
                // 6-series: 'A' marks the a=1 mean line, '=' an "a=0.x" suffix.
                let (mut ieql, mut ipoint) = (false, false);
                for col in 10..=80 {
                    let c = self.col(col);
                    icount += 1;
                    if icount > 8 {
                        break;
                    }
                    if c.is_ascii_digit() {
                        self.set_digit(icount, (c - b'0') as i32)?;
                    }
                    if icount == 3 && c == b'A' {
                        icount += 1;
                    }
                    if icount == 4 && c == b'A' {
                        self.set_digit(icount, 1)?;
                    }
                    if icount > 4 && c == b'A' {
                        icount -= 1;
                    }
                    if c == b' ' {
                        icount -= 1;
                    }
                    if c == b',' {
                        icount -= 1;
                    }
                    if c == b'-' && icount == 3 {
                        icount += 1;
                    }
                    if c == b'=' {
                        ieql = true;
                    }
                    if ieql && !ipoint {
                        icount -= 1;
                    }
                    if c == b' ' && ieql && !ipoint {
                        icount += 1;
                    }
                    if c == b'.' {
                        ipoint = true;
                    }
                }
            }
            7 => self.decode_supersonic()?,
            _ => {
                // 4/5-digit and 1-series; a '-' selects the modified family.
                for col in 10..=80 {
                    let c = self.col(col);
                    icount += 1;
                    if icount > 10 {
                        break;
                    }
                    if c.is_ascii_digit() {
                        self.set_digit(icount, (c - b'0') as i32)?;
                    }
                    if c == b' ' {
                        icount -= 1;
                    }
                    if c == b'-' && na != 5 {
                        na += 1;
                    }
                    if c == b'.' {
                        icount -= 1;
                    }
                }
            }
        }
        // Grid: X(1)=0, steps of .001 to 1%, .01 to 30%, .05 to 80%, then .02.
        let mut delx: f32 = 0.001;
        self.x[0] = 0.0;
        let mut lll = 1;
        for li in 1..NP {
            lll = li + 1;
            self.x[li] = self.x[li - 1] + delx;
            if self.x[li] >= 0.01 {
                delx = 0.01;
            }
            if self.x[li] > 0.29 {
                delx = 0.05;
            }
            if self.x[li] > 0.79 {
                delx = 0.02;
            }
            if self.x[li] >= 1.0 {
                break;
            }
        }
        self.x[lll - 1] = 1.0;
        self.n = lll;
        Ok(na)
    }

    /// DECODE's NACA S-designation parser: three "d-dd.d" style fields, each
    /// normalised to three digits (I J K II ... LLL).
    fn decode_supersonic(&mut self) -> Result<()> {
        let mut icount: i32 = 0;
        let mut ncount: i32 = 2;
        for col in 10..=80 {
            if icount == 10 {
                continue;
            }
            let c = self.col(col);
            // Labels 1090 (field end) and 1100 (decimal point) of the original.
            let mut end_field = false;
            let mut point = false;
            if c == b' ' {
                if col == 80 {
                    end_field = true;
                } else {
                    continue;
                }
            } else if c.is_ascii_digit() {
                ncount += 1;
                if ncount > 3 {
                    continue;
                }
                icount += 1;
                self.set_digit(icount, (c - b'0') as i32)?;
                // Label 1080: only the last column closes a field.
                if col == 80 {
                    end_field = true;
                } else {
                    continue;
                }
            } else if c == b'-' && icount > 0 {
                end_field = true;
            } else if c == b'.' && icount > 0 {
                point = true;
            } else if col == 80 {
                end_field = true;
            } else {
                continue;
            }
            if end_field {
                if ncount == 0 {
                    icount += 3;
                }
                if ncount == 1 {
                    let v = self.digit(icount)?;
                    self.set_digit(icount + 1, v)?;
                    self.set_digit(icount, 0)?;
                    icount += 2;
                }
                if ncount == 2 {
                    icount += 1;
                }
                if icount == 1 {
                    ncount = 0;
                }
                if ncount > 0 {
                    ncount = 0;
                }
            }
            if point {
                if ncount == 0 {
                    icount += 2;
                    ncount = 2;
                }
                if ncount == 1 {
                    let v = self.digit(icount)?;
                    self.set_digit(icount + 1, v)?;
                    self.set_digit(icount, 0)?;
                    icount += 1;
                    ncount = 2;
                }
            }
        }
        Ok(())
    }

    /// AIRFOL: decode the card and generate the section geometry.
    pub(super) fn airfol(&mut self, case: &mut Case, view: &SlotView) -> Result<()> {
        let na = self.decode()?;
        match na {
            2 => self.cord4m(),
            3 => self.coord5(),
            4 => self.cord5m(),
            5 => self.coord1(),
            6 => self.coord6(),
            7 => {
                // S-2 (biconvex) always has its maximum thickness at 50%.
                if self.digits[0] == 2 {
                    self.digits[1] = 5;
                    self.digits[2] = 0;
                    self.digits[3] = 0;
                }
                self.cordsp(case, view);
                self.xycord(0)
            }
            // NA 1, and the computed GO TO's fall-through for any other value.
            _ => self.coord4(),
        }
    }
}
