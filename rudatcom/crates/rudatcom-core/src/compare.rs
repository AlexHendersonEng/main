//! Comparison of two `datcom.out`-style reports.
//!
//! Rule: text layout must match exactly; numbers may differ within a relative/absolute
//! tolerance. Used by the golden-file tests (and available for `rudatcom check` later).

/// Tolerances for numeric comparison.
#[derive(Debug, Clone, Copy)]
pub struct Tolerance {
    /// Relative tolerance, scaled by the larger magnitude.
    pub rel: f64,
    /// Absolute tolerance, so values near zero (e.g. 1e-30 vs 0) still match.
    pub abs: f64,
}

impl Default for Tolerance {
    fn default() -> Self {
        Self {
            rel: 1e-4,
            abs: 1e-6,
        }
    }
}

/// One piece of a line: literal text, or a number with the padding before it.
#[derive(Debug, PartialEq)]
enum Piece<'a> {
    Text(&'a str),
    /// `pad` is the whitespace run before the number; `raw` is the number text.
    Number {
        pad: &'a str,
        raw: &'a str,
        value: f64,
    },
}

fn is_word(c: u8) -> bool {
    c.is_ascii_alphanumeric() || c == b'_'
}

/// Length of the number starting at `s[i]`, if one starts there.
fn number_len(s: &[u8], i: usize) -> Option<usize> {
    let mut j = i;
    if matches!(s[j], b'+' | b'-') {
        j += 1;
    }
    let int_start = j;
    while j < s.len() && s[j].is_ascii_digit() {
        j += 1;
    }
    let mut digits = j - int_start;
    if j < s.len() && s[j] == b'.' {
        j += 1;
        let frac_start = j;
        while j < s.len() && s[j].is_ascii_digit() {
            j += 1;
        }
        digits += j - frac_start;
    }
    if digits == 0 {
        return None;
    }
    // Optional exponent, e.g. E-30 / D+02; only consumed when well formed.
    if j < s.len() && matches!(s[j], b'E' | b'e' | b'D' | b'd') {
        let mut k = j + 1;
        if k < s.len() && matches!(s[k], b'+' | b'-') {
            k += 1;
        }
        let exp_start = k;
        while k < s.len() && s[k].is_ascii_digit() {
            k += 1;
        }
        if k > exp_start {
            j = k;
        }
    }
    Some(j - i)
}

/// Split a line into text and number pieces.
fn split_line(line: &str) -> Vec<Piece<'_>> {
    let b = line.as_bytes();
    let mut pieces = Vec::new();
    let (mut text_start, mut i) = (0, 0);
    while i < b.len() {
        let starts_number = (b[i].is_ascii_digit() || matches!(b[i], b'+' | b'-' | b'.'))
            && (i == 0 || !is_word(b[i - 1]));
        if starts_number {
            if let Some(len) = number_len(b, i) {
                // Absorb preceding spaces into the number so sign/padding shifts are tolerated
                // as long as the overall field width is unchanged.
                let mut pad_start = i;
                while pad_start > text_start && b[pad_start - 1] == b' ' {
                    pad_start -= 1;
                }
                if pad_start > text_start {
                    pieces.push(Piece::Text(&line[text_start..pad_start]));
                }
                let raw = &line[i..i + len];
                let value = raw
                    .replace(['D', 'd'], "E")
                    .parse::<f64>()
                    .unwrap_or(f64::NAN);
                pieces.push(Piece::Number {
                    pad: &line[pad_start..i],
                    raw,
                    value,
                });
                i += len;
                text_start = i;
                continue;
            }
        }
        i += 1;
    }
    if text_start < b.len() {
        pieces.push(Piece::Text(&line[text_start..]));
    }
    pieces
}

/// Value of one unit in the last printed digit of `raw` (e.g. `4.660E-02` -> 1e-5).
fn last_digit_unit(raw: &str) -> f64 {
    let upper = raw.to_ascii_uppercase().replace('D', "E");
    let (mantissa, exp) = upper.split_once('E').unwrap_or((&upper, "0"));
    let decimals = mantissa.split_once('.').map_or(0, |(_, f)| f.len()) as i32;
    10f64.powi(exp.parse::<i32>().unwrap_or(0) - decimals)
}

/// Numbers match within the tolerance, or within 1.5 units of the last printed digit
/// (the printed values are rounded, so the last digit can legitimately flip).
fn numbers_match(a: f64, b: f64, raw_a: &str, tol: Tolerance) -> bool {
    let diff = (a - b).abs();
    diff <= tol.abs + tol.rel * a.abs().max(b.abs()) || diff <= 1.5 * last_digit_unit(raw_a)
}

/// Compare one line; returns a reason on mismatch.
fn compare_line(expected: &str, actual: &str, tol: Tolerance) -> Option<String> {
    let (e, a) = (split_line(expected), split_line(actual));
    if e.len() != a.len() {
        return Some("layout differs".into());
    }
    for (pe, pa) in e.iter().zip(&a) {
        match (pe, pa) {
            (Piece::Text(x), Piece::Text(y)) if x == y => {}
            (
                Piece::Number {
                    pad: p1,
                    raw: r1,
                    value: v1,
                },
                Piece::Number {
                    pad: p2,
                    raw: r2,
                    value: v2,
                },
            ) => {
                if p1.len() + r1.len() != p2.len() + r2.len() {
                    return Some("number field width differs".into());
                }
                if !numbers_match(*v1, *v2, r1, tol) {
                    return Some(format!("numbers differ: {r1} vs {r2}"));
                }
            }
            _ => return Some("text differs".into()),
        }
    }
    None
}

/// Compare two reports. Returns `Err` with a readable list of the first mismatches.
pub fn compare_reports(expected: &str, actual: &str, tol: Tolerance) -> Result<(), String> {
    const MAX_REPORTED: usize = 10;
    let (exp, act): (Vec<&str>, Vec<&str>) = (expected.lines().collect(), actual.lines().collect());
    let mut problems = Vec::new();
    let mut total = 0;
    if exp.len() != act.len() {
        problems.push(format!(
            "line count differs: expected {}, got {}",
            exp.len(),
            act.len()
        ));
        total += 1;
    }
    for (n, (e, a)) in exp.iter().zip(&act).enumerate() {
        if let Some(why) = compare_line(e.trim_end(), a.trim_end(), tol) {
            total += 1;
            if problems.len() < MAX_REPORTED {
                problems.push(format!(
                    "line {} ({why})\n  expected: {e}\n  actual:   {a}",
                    n + 1
                ));
            }
        }
    }
    if total == 0 {
        Ok(())
    } else {
        Err(format!(
            "{total} mismatching line(s):\n{}",
            problems.join("\n")
        ))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn same(a: &str, b: &str) -> bool {
        compare_reports(a, b, Tolerance::default()).is_ok()
    }

    #[test]
    fn identical_text_matches() {
        assert!(same(" ALPHA = 1.0\n", " ALPHA = 1.0\n"));
    }

    #[test]
    fn small_numeric_difference_is_tolerated() {
        assert!(same("CL= 1.00000E+00", "CL= 1.00001E+00"));
    }

    #[test]
    fn large_numeric_difference_fails() {
        assert!(!same("CL= 1.0", "CL= 1.5"));
    }

    #[test]
    fn last_printed_digit_flip_is_tolerated() {
        assert!(same("X= 4.660E-02", "X= 4.661E-02"));
        assert!(!same("X= 4.660E-02", "X= 4.670E-02"));
    }

    #[test]
    fn negative_zero_matches_zero_within_same_width() {
        assert!(same("BD(195)= 0.00000E+00", "BD(195)=-0.00000E+00"));
    }

    #[test]
    fn tiny_values_match_zero() {
        assert!(same("X= 1.00000E-30", "X= 0.00000E+00"));
    }

    #[test]
    fn text_difference_fails() {
        assert!(!same("CASE 1", "CASE X"));
    }

    #[test]
    fn field_width_change_fails() {
        assert!(!same("A= 1.0 B", "A=1.0 B"));
    }

    #[test]
    fn line_count_difference_fails() {
        assert!(!same("a\nb", "a"));
    }

    #[test]
    fn identifiers_with_digits_are_text() {
        assert!(!same("EX1", "EX2"));
    }
}
