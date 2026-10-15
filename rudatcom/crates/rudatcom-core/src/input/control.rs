//! Control cards: port of INPUTC (and the card recognition of CCARD).

use super::{Card, CaseBuilder, Diagnostic, Slot, CARD_WIDTH};

/// Unit system selected by the `DIM` card (the original's `IDIM`, 1-4).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DimUnit {
    Feet = 1,
    Inches = 2,
    Meters = 3,
    Centimeters = 4,
}

/// Flags set by control cards for one case.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Controls {
    /// `SAVE`: keep this case's namelists for the next case.
    pub save: bool,
    pub trim: bool,
    pub damp: bool,
    pub part: bool,
    pub build: bool,
    pub plot: bool,
    /// `NAMELIST`: list the final inputs.
    pub list_inputs: bool,
    /// `PRINT EXTRAP`.
    pub print_extrap: bool,
    /// `DERIV RAD` (true) / `DERIV DEG` (false); `None` if never given.
    pub deriv_rad: Option<bool>,
    pub dump_all: bool,
    pub dump_case: bool,
    /// Variable names requested with `DUMP name ...`.
    pub dump_vars: Vec<String>,
}

pub(super) enum Flow {
    Continue,
    EndCase,
}

fn starts(card: &Card, s: &str) -> bool {
    card.b.starts_with(s.as_bytes())
}

/// Apply one control card to the case being built.
pub(super) fn apply(card: &Card, b: &mut CaseBuilder, dim: &mut DimUnit) -> Flow {
    let c = &mut b.controls;
    if starts(card, "NEXT CASE") {
        return Flow::EndCase;
    }
    if starts(card, "DUMP ALL") && card.b[8..].iter().all(|x| *x == b' ') {
        c.dump_all = true;
    } else if starts(card, "DUMP ") {
        if card.b[5..9] == *b"CASE" && card.b[9..].iter().all(|x| *x == b' ') {
            c.dump_case = true;
        } else {
            // Names are separated by blanks or commas; each is at most 6 characters.
            let text = String::from_utf8_lossy(&card.b[5..]).into_owned();
            for tok in text.split([' ', ',']).filter(|t| !t.is_empty()) {
                if tok.len() > 6 {
                    b.diagnostics.push(Diagnostic {
                        line: card.line,
                        message: format!("CONTAINS ILLEGAL VARIABLE NAME TO BE DUMPED: {tok}"),
                        card: card.text(),
                    });
                } else {
                    c.dump_vars.push(tok.to_string());
                }
            }
        }
    } else if starts(card, "SAVE") {
        c.save = true;
    } else if starts(card, "CASEID") {
        let id = String::from_utf8_lossy(&card.b[6..CARD_WIDTH])
            .trim_end()
            .to_string();
        b.id = Some(id);
    } else if starts(card, "TRIM") {
        c.trim = true;
    } else if starts(card, "DAMP") {
        c.damp = true;
    } else if starts(card, "PART") {
        c.part = true;
    } else if starts(card, "PRINT EXTRAP") {
        c.print_extrap = true;
    } else if starts(card, "BUILD") {
        c.build = true;
    } else if starts(card, "NACA") {
        // Kept in the namelist stream so it is applied in input order.
        b.order.push(Slot::Naca);
        b.cards.push(*card);
        b.in_expr = false;
        let surface = card.b[5] as char;
        if !"WHVF".contains(surface) {
            b.diagnostics.push(Diagnostic {
                line: card.line,
                message: "INCORRECT LIFTING SURFACE DESIGNATION ON NACA CARD".into(),
                card: card.text(),
            });
        }
    } else if starts(card, "DERIV DEG") {
        c.deriv_rad = Some(false);
    } else if starts(card, "DERIV RAD") {
        c.deriv_rad = Some(true);
    } else if starts(card, "DIM") {
        let unit = match &card.b[4..6] {
            b"FT" => Some(DimUnit::Feet),
            b"IN" => Some(DimUnit::Inches),
            b"M " => Some(DimUnit::Meters),
            b"CM" => Some(DimUnit::Centimeters),
            _ => None,
        };
        match unit {
            Some(u) => *dim = u,
            None => bad_card(b, card),
        }
    } else if starts(card, "NAMELIST") {
        c.list_inputs = true;
    } else if starts(card, "PLOT") {
        c.plot = true;
    } else {
        bad_card(b, card);
    }
    Flow::Continue
}

fn bad_card(b: &mut CaseBuilder, card: &Card) {
    b.diagnostics.push(Diagnostic {
        line: card.line,
        message: "ERROR IN CARD BELOW".into(),
        card: card.text(),
    });
}
