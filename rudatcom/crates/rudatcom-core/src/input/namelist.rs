//! Namelist emulator: port of NAMER and its helpers (TODEC, TOINT, TOLOG, REPTCT).
//!
//! Columns are 0-based here; the original's "column 81" (not found) is `CARD_WIDTH`.

use super::{Card, Namelist, CARD_WIDTH};
use crate::Error;

/// Sequential reader over the unit-9 card stream (never rewound between namelists).
pub(crate) struct Cursor<'a> {
    cards: &'a [Card],
    pos: usize,
}

impl<'a> Cursor<'a> {
    pub fn new(cards: &'a [Card]) -> Self {
        Self { cards, pos: 0 }
    }

    pub fn next_card(&mut self) -> Option<Card> {
        let c = self.cards.get(self.pos).copied();
        self.pos += 1;
        c
    }
}

/// FINDCH: first column >= `from` holding `ch`, or `CARD_WIDTH`.
fn find(kol: &[u8; CARD_WIDTH], ch: u8, from: usize) -> usize {
    (from.min(CARD_WIDTH)..CARD_WIDTH)
        .find(|&i| kol[i] == ch)
        .unwrap_or(CARD_WIDTH)
}

/// SKIPBL: first non-blank column >= `from`, or `CARD_WIDTH`.
fn skip_blank(kol: &[u8; CARD_WIDTH], from: usize) -> usize {
    (from.min(CARD_WIDTH)..CARD_WIDTH)
        .find(|&i| kol[i] != b' ')
        .unwrap_or(CARD_WIDTH)
}

/// libgcc `__powisf2`: gfortran evaluates `real ** integer` with this exact algorithm,
/// so using it keeps number conversion bit-identical to the original.
fn powi(mut x: f32, m: i32) -> f32 {
    let mut n = m.unsigned_abs();
    let mut y = if n % 2 == 1 { x } else { 1.0 };
    loop {
        n >>= 1;
        if n == 0 {
            break;
        }
        x *= x;
        if n % 2 == 1 {
            y *= x;
        }
    }
    if m < 0 {
        1.0 / y
    } else {
        y
    }
}

/// TODEC: convert a numeric string to f32 using integer digit accumulation.
/// Returns `(value, error)`; `error` means a character that is not part of a number.
pub(crate) fn to_dec(s: &[u8]) -> (f32, bool) {
    let at = |i: usize| s.get(i).copied().unwrap_or(b' ');
    let (mut lvalue, mut rvalue, mut power): (i32, i32, i32) = (0, 0, 0);
    let (mut signv, mut signp, mut pten) = (0.0f32, 0i32, 0i32);
    let mut frac = false;
    let mut err = false;
    let mut exponent_at = None;

    for i in 0..CARD_WIDTH {
        match at(i) {
            b' ' => {}
            b'E' => {
                exponent_at = Some(i + 1);
                break;
            }
            b'+' | b'-' => {
                if signv != 0.0 {
                    break;
                }
                signv = if at(i) == b'+' { 1.0 } else { -1.0 };
            }
            b'.' => frac = true,
            c if c.is_ascii_digit() => {
                let d = i32::from(c - b'0');
                if frac {
                    rvalue = rvalue.wrapping_mul(10).wrapping_add(d);
                    pten += 1;
                } else {
                    lvalue = lvalue.wrapping_mul(10).wrapping_add(d);
                }
            }
            _ => {
                err = true;
                break;
            }
        }
    }

    if let Some(start) = exponent_at {
        for i in start..CARD_WIDTH {
            match at(i) {
                b'+' | b'-' => {
                    if signp != 0 {
                        break;
                    }
                    signp = if at(i) == b'+' { 1 } else { -1 };
                }
                c if c.is_ascii_digit() => {
                    power = power.wrapping_mul(10).wrapping_add(i32::from(c - b'0'));
                }
                _ => break,
            }
        }
    }

    if signv == 0.0 {
        signv = 1.0;
    }
    if signp == 0 {
        signp = 1;
    }
    let mantissa = lvalue as f32 + rvalue as f32 * powi(10.0, -pten);
    (
        signv * mantissa * powi(10.0, signp.wrapping_mul(power)),
        err,
    )
}

/// TOINT: like `to_dec` but rejects a decimal point; rounds to nearest.
pub(crate) fn to_int(s: &[u8]) -> (i32, bool) {
    let has_point = s.contains(&b'.');
    let (ans, err) = to_dec(s);
    let sign = if ans < 0.0 { -1.0 } else { 1.0 };
    ((sign * (ans.abs() + 0.5)) as i32, has_point || err)
}

/// TOLOG: `.T.`, `.TRUE.`, `.F.`, `.FALSE.` (leading blanks allowed).
pub(crate) fn to_log(s: &[u8]) -> Option<bool> {
    let at = |i: usize| s.get(i).copied().unwrap_or(b' ');
    let start = (0..CARD_WIDTH).find(|&i| at(i) != b' ')?;
    if at(start) != b'.' {
        return None;
    }
    let word = |w: &[u8]| {
        (0..w.len()).all(|k| at(start + 1 + k) == w[k]) && at(start + 1 + w.len()) == b'.'
    };
    if word(b"T") || word(b"TRUE") {
        Some(true)
    } else if word(b"F") || word(b"FALSE") {
        Some(false)
    } else {
        None
    }
}

/// REPTCT: split an optional `n*` repeat count off a constant.  Returns `None` on a bad count.
fn split_repeat(text: &[u8]) -> Option<(usize, &[u8])> {
    match text.iter().position(|c| *c == b'*') {
        None => Some((1, text)),
        Some(star) => {
            let (n, err) = to_int(&text[..star]);
            if err || n < 0 {
                return None;
            }
            // A zero count keeps the default of 1, as in the original.
            Some((if n == 0 { 1 } else { n as usize }, &text[star + 1..]))
        }
    }
}

fn fatal(card: &Card, message: &str) -> Error {
    Error::Parse {
        line: card.line,
        message: message.to_string(),
        card: card.text(),
    }
}

const NUMERIC_START: &[u8] = b"0123456789+-.";

/// NAMER: scan the stream for `$NAME` and read its variable assignments into `target`.
pub(crate) fn read_namelist(cur: &mut Cursor<'_>, target: &mut Namelist) -> Result<(), Error> {
    let def = target.def;

    // Stage 1/2: find the card that starts the wanted namelist.
    let mut card;
    let mut icol;
    'find: loop {
        card = match cur.next_card() {
            Some(c) => c,
            None => return Ok(()),
        };
        icol = skip_blank(&card.b, 0);
        if icol >= CARD_WIDTH || card.b[icol] != b'$' {
            continue;
        }
        icol += 1;
        let lcol = find(&card.b, b' ', icol);
        if lcol == icol {
            return Err(fatal(
                &card,
                "NAMELIST INPUT ERROR: ILLEGAL/INCORRECT SPECIFICATION OF NAMELIST NAME",
            ));
        }
        if &card.b[icol..lcol] == def.name.as_bytes() {
            icol = lcol;
            break 'find;
        }
        // Wrong namelist: skip cards until a `$` terminator is seen.
        let mut k = icol;
        let mut c = card;
        loop {
            if find(&c.b, b'$', k) < CARD_WIDTH {
                continue 'find;
            }
            c = match cur.next_card() {
                Some(n) => n,
                None => return Ok(()),
            };
            k = 0;
        }
    }

    let mut iend = false;
    'vars: loop {
        // Stage 3: variable name up to `=`.
        icol = skip_blank(&card.b, icol);
        while icol >= CARD_WIDTH {
            card = match cur.next_card() {
                Some(c) => c,
                None => return Ok(()),
            };
            icol = skip_blank(&card.b, 0);
        }
        if card.b[icol] == b'$' || iend {
            return Ok(());
        }
        let lcol = find(&card.b, b'=', icol);
        if lcol >= CARD_WIDTH {
            return Err(fatal(
                &card,
                "NAMELIST INPUT ERROR: NO EQUALS FOLLOWING VARIABLE NAME.",
            ));
        }
        let mut name: Vec<u8> = card.b[icol..lcol].to_vec();
        let mut ioff = 0usize;
        if let Some(kk) = name.iter().position(|c| *c == b'(') {
            let ll =
                match name.iter().skip(kk).position(|c| *c == b')') {
                    Some(p) => kk + p,
                    None => return Err(fatal(
                        &card,
                        "NAMELIST INPUT ERROR: NO CLOSING RIGHT PARENTHESIS IN ARRAY DEFINITION.",
                    )),
                };
            let (sub, err) = to_int(&name[kk + 1..ll]);
            if err || sub < 1 {
                return Err(fatal(
                    &card,
                    "NAMELIST INPUT ERROR: ILLEGAL/INVALID ARRAY SUBSCRIPT.",
                ));
            }
            ioff = sub as usize - 1;
            name.drain(kk..=ll);
        }
        let name_str = String::from_utf8_lossy(&name).trim_end().to_string();
        icol = lcol + 1;

        // Stage 4-6: one or more constants (extra constants continue the array).
        let mut search = true;
        let mut var_idx = None;
        loop {
            if icol < CARD_WIDTH && card.b[icol] == b'$' {
                iend = true;
            }
            let mut end = find(&card.b, b',', icol);
            if end >= CARD_WIDTH {
                end = find(&card.b, b'$', icol);
                if end < CARD_WIDTH {
                    iend = true;
                }
            }
            let text = &card.b[icol.min(CARD_WIDTH)..end.max(icol.min(CARD_WIDTH))];
            let (repeat, text) = split_repeat(text).ok_or_else(|| {
                fatal(
                    &card,
                    "NAMELIST INPUT ERROR: CONSTANT DOES NOT MATCH TYPE OF INPUT REQUIRED.",
                )
            })?;
            icol = end + 1;

            if search {
                var_idx = def.vars.iter().position(|v| v.name == name_str);
                if var_idx.is_none() {
                    return Err(fatal(
                        &card,
                        "NAMELIST INPUT ERROR: VARIABLE NAME NOT IN NAMELIST.",
                    ));
                }
            }
            let var = &def.vars[var_idx.unwrap_or(0)];
            if ioff + 1 > var.dim {
                return Err(fatal(
                    &card,
                    "NAMELIST INPUT ERROR: ILLEGAL/INVALID ARRAY SUBSCRIPT.",
                ));
            }

            let value = if var.logical {
                to_log(text).map(|b| if b { 1.0 } else { 0.0 })
            } else {
                let (v, err) = to_dec(text);
                (!err).then_some(v)
            };
            let value = value.ok_or_else(|| {
                fatal(
                    &card,
                    "NAMELIST INPUT ERROR: CONSTANT DOES NOT MATCH TYPE OF INPUT REQUIRED.",
                )
            })?;

            for i in 1..=repeat {
                if i + ioff > var.dim {
                    return Err(fatal(
                        &card,
                        "NAMELIST INPUT ERROR: REPEAT COUNT EXCEEDS VARIABLE ARRAY DIMENSION.",
                    ));
                }
                if var.loc >= 1 {
                    let ii = var.loc + i + ioff - 1;
                    if ii > def.size {
                        return Err(fatal(
                            &card,
                            "NAMELIST INPUT ERROR: ARRAY SUBSCRIPT OR NUMBER OF CONSTANTS EXCEEDS VARIABLE DIMENSION OR COMMON BLOCK SIZE.",
                        ));
                    }
                    target.values[ii - 1] = value;
                }
            }
            ioff += repeat;

            // A following number or logical continues the same array; anything else
            // starts the next variable.
            icol = skip_blank(&card.b, icol);
            if iend {
                return Ok(());
            }
            while icol >= CARD_WIDTH {
                card = match cur.next_card() {
                    Some(c) => c,
                    None => return Ok(()),
                };
                icol = skip_blank(&card.b, 0);
            }
            if NUMERIC_START.contains(&card.b[icol]) {
                search = false;
            } else {
                continue 'vars;
            }
        }
    }
}
