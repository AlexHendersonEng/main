//! Input deck reader: port of the original CONERR/INPUT/INPUTB/INPUTC/NAMER pipeline.
//!
//! A deck is a sequence of 80-column cards.  A card with a non-blank column 1 is a
//! control card (`NEXT CASE`, `SAVE`, `CASEID`, ...).  Any other card belongs to the
//! `$NAME ... $` namelist stream.  [`parse_deck`] splits a deck into [`Case`]s and
//! applies each namelist onto flat `f32` storage laid out exactly like the original
//! COMMON blocks (see [`tables`]), so later ports can index it by the same offsets.

mod control;
mod finalize;
mod namelist;
mod tables;

use crate::Error;
pub use control::{Controls, DimUnit};
pub use finalize::{finalize_case, Regimes};
pub use tables::NAMELISTS;

/// Sentinel for "variable not supplied" (`UNUSED` in the Fortran CONSNT block).
pub const UNUSED: f32 = 1.0e-30;
/// Width of an input card.
pub const CARD_WIDTH: usize = 80;

/// One namelist variable (generated from the XNAMn tables).
#[derive(Debug)]
pub struct VarDef {
    pub name: &'static str,
    /// Number of elements (1 for scalars).
    pub dim: usize,
    /// Logical (`.TRUE.`/`.FALSE.`) rather than real.
    pub logical: bool,
    /// 1-based offset into the namelist's storage array; 0 = read but not stored.
    pub loc: usize,
}

/// One namelist definition (generated from the XNAMn tables).
#[derive(Debug)]
pub struct NamelistDef {
    pub name: &'static str,
    /// Length of the backing storage array (MAXCOM in NAMER).
    pub size: usize,
    pub vars: &'static [VarDef],
}

/// Look up a namelist definition by name.
pub fn namelist_def(name: &str) -> Option<&'static NamelistDef> {
    NAMELISTS.iter().find(|d| d.name == name)
}

/// Values of one namelist.  Unset reals hold [`UNUSED`]; logicals are stored as 1.0/0.0.
/// Aliased variables (e.g. `ALPHA` and `ALSCHD`) share storage because they share `loc`.
#[derive(Debug, Clone)]
pub struct Namelist {
    pub def: &'static NamelistDef,
    pub values: Vec<f32>,
    /// True once a `$NAME` group for this namelist has been read.
    pub present: bool,
}

impl Namelist {
    pub fn new(def: &'static NamelistDef) -> Self {
        Self {
            def,
            values: vec![UNUSED; def.size],
            present: false,
        }
    }

    fn var(&self, name: &str) -> Option<&'static VarDef> {
        self.def.vars.iter().find(|v| v.name == name)
    }

    /// All elements of a variable, or `None` if the name is unknown or not stored.
    pub fn get(&self, name: &str) -> Option<&[f32]> {
        let v = self.var(name)?;
        if v.loc == 0 {
            return None;
        }
        let start = v.loc - 1;
        self.values
            .get(start..(start + v.dim).min(self.values.len()))
    }

    /// First element of a variable if it was supplied (not [`UNUSED`]).
    pub fn scalar(&self, name: &str) -> Option<f32> {
        self.get(name)?.first().copied().filter(|v| *v != UNUSED)
    }

    /// Logical variable value (false when unset).
    pub fn flag(&self, name: &str) -> bool {
        self.get(name)
            .and_then(|v| v.first())
            .is_some_and(|v| *v == 1.0)
    }
}

/// A problem found in the deck, with its 1-based source line.
#[derive(Debug, Clone, PartialEq)]
pub struct Diagnostic {
    pub line: usize,
    pub message: String,
    /// The offending card text (trailing blanks removed).
    pub card: String,
}

/// An experimental-data namelist (`$EXPRnn`).  The original diverts these cards to a side
/// file; they are kept raw until the experimental-data substitution is ported.
#[derive(Debug, Clone, PartialEq)]
pub struct ExprGroup {
    /// The two-digit suffix of `$EXPRnn`.
    pub index: u8,
    pub cards: Vec<String>,
}

/// A `NACA-x-...` control card (treated by the original as a pseudo-namelist).
#[derive(Debug, Clone, PartialEq)]
pub struct NacaCard {
    /// Surface designator from column 6: `W`, `H`, `V` or `F`.
    pub surface: char,
    pub text: String,
}

/// One DATCOM case: everything between `NEXT CASE` cards.
#[derive(Debug, Clone)]
pub struct Case {
    pub id: Option<String>,
    pub controls: Controls,
    /// Unit system in effect (`DIM` card; persists across cases like the original).
    pub dim: DimUnit,
    /// One entry per namelist, in the same order as [`NAMELISTS`].
    pub namelists: Vec<Namelist>,
    pub naca: Vec<NacaCard>,
    pub expr: Vec<ExprGroup>,
    /// Non-fatal problems; the original refuses to run a case that has any errors.
    pub diagnostics: Vec<Diagnostic>,
    /// Messages the original prints but still runs the case.
    pub warnings: Vec<Diagnostic>,
    /// Speed regimes implied by the Mach numbers (INPUTB).
    pub regimes: Regimes,
}

impl Case {
    /// Namelist by name, e.g. `case.namelist("FLTCON")`.
    pub fn namelist(&self, name: &str) -> Option<&Namelist> {
        self.namelists.iter().find(|n| n.def.name == name)
    }

    /// True when the case has no input errors (the original's `GONOGO` flag).
    pub fn is_runnable(&self) -> bool {
        self.diagnostics.is_empty()
    }
}

/// A whole input deck.
#[derive(Debug, Clone)]
pub struct Deck {
    pub cases: Vec<Case>,
}

/// An 80-column card with its source line number.
#[derive(Debug, Clone, Copy)]
pub(crate) struct Card {
    pub line: usize,
    pub b: [u8; CARD_WIDTH],
}

impl Card {
    fn new(line: usize, text: &str) -> Self {
        let mut b = [b' '; CARD_WIDTH];
        // Non-ASCII bytes are replaced so every column is one byte.
        for (slot, ch) in b.iter_mut().zip(text.chars()) {
            *slot = if ch.is_ascii() { ch as u8 } else { b'?' };
        }
        Self { line, b }
    }

    pub(crate) fn text(&self) -> String {
        String::from_utf8_lossy(&self.b).trim_end().to_string()
    }
}

/// What the unit-9 stream holds for each recognised namelist, in input order.
#[derive(Debug, Clone, Copy)]
enum Slot {
    Namelist(usize),
    Naca,
}

/// State accumulated while reading one case (the original's unit 9 + NAMORD arrays).
#[derive(Default, Clone)]
struct CaseBuilder {
    cards: Vec<Card>,
    order: Vec<Slot>,
    controls: Controls,
    id: Option<String>,
    expr: Vec<ExprGroup>,
    diagnostics: Vec<Diagnostic>,
    /// In the middle of an `$EXPRnn` group (its continuation cards go to `expr`).
    in_expr: bool,
}

/// Parse a complete input deck.  Fatal namelist syntax errors (which stop the original)
/// are returned as [`Error::Parse`]; per-case problems are kept in [`Case::diagnostics`].
pub fn parse_deck(text: &str) -> Result<Deck, Error> {
    let mut cases = Vec::new();
    let mut dim = DimUnit::Feet;
    let mut deriv_rad = None;
    let mut b = CaseBuilder::default();

    for (i, raw) in text.lines().enumerate() {
        let card = Card::new(i + 1, raw.trim_end_matches('\r'));
        if card.b[0] != b' ' {
            // Control card.
            match control::apply(&card, &mut b, &mut dim) {
                control::Flow::Continue => {}
                control::Flow::EndCase => {
                    // DERIV DEG/RAD persists into later cases, like the original's DRCONV.
                    if b.controls.deriv_rad.is_none() {
                        b.controls.deriv_rad = deriv_rad;
                    }
                    deriv_rad = b.controls.deriv_rad;
                    cases.push(finish_case(b.clone(), dim)?);
                    b = next_builder(&b);
                }
            }
        } else {
            add_namelist_card(&mut b, card);
        }
    }
    // End of file: the last case only counts if it has namelist data.
    if !b.order.is_empty() || !b.expr.is_empty() {
        if b.controls.deriv_rad.is_none() {
            b.controls.deriv_rad = deriv_rad;
        }
        cases.push(finish_case(b, dim)?);
    }
    Ok(Deck { cases })
}

/// Start the next case; after `SAVE` the namelist stream is carried over.
fn next_builder(prev: &CaseBuilder) -> CaseBuilder {
    if prev.controls.save {
        CaseBuilder {
            cards: prev.cards.clone(),
            order: prev.order.clone(),
            expr: prev.expr.clone(),
            ..Default::default()
        }
    } else {
        CaseBuilder::default()
    }
}

/// Classify a non-control card (port of the namelist branch of INPUT).
fn add_namelist_card(b: &mut CaseBuilder, card: Card) {
    if card.b[1] == b'$' && !card.b[2..].starts_with(b"END") {
        let name = card_name(&card);
        if let Some(num) = name.strip_prefix("EXPR") {
            // `$EXPRnn`: exactly two digits in columns 7-8.
            if num.len() >= 2 && num.as_bytes()[..2].iter().all(u8::is_ascii_digit) {
                let index = num[..2].parse().unwrap_or(0);
                b.expr.push(ExprGroup {
                    index,
                    cards: vec![card.text()],
                });
                b.in_expr = true;
            } else {
                b.diagnostics.push(Diagnostic {
                    line: card.line,
                    message: "ERROR IN CARD BELOW".into(),
                    card: card.text(),
                });
                b.in_expr = false;
            }
            return;
        }
        b.in_expr = false;
        match NAMELISTS.iter().position(|d| d.name == name) {
            Some(idx) => {
                b.order.push(Slot::Namelist(idx));
                b.cards.push(card);
            }
            None => b.diagnostics.push(Diagnostic {
                line: card.line,
                message: "ERROR IN CARD BELOW".into(),
                card: card.text(),
            }),
        }
        return;
    }
    if b.in_expr {
        if let Some(g) = b.expr.last_mut() {
            g.cards.push(card.text());
        }
    } else {
        b.cards.push(card);
    }
}

/// The namelist name following `$` in column 2 (up to the first blank).
fn card_name(card: &Card) -> String {
    let rest = &card.b[2..];
    let end = rest.iter().position(|c| *c == b' ').unwrap_or(rest.len());
    String::from_utf8_lossy(&rest[..end]).into_owned()
}

/// Replay the namelist stream onto fresh storage and post-process (INPUTB).
fn finish_case(b: CaseBuilder, dim: DimUnit) -> Result<Case, Error> {
    let mut namelists: Vec<Namelist> = NAMELISTS.iter().map(Namelist::new).collect();
    let mut cursor = namelist::Cursor::new(&b.cards);
    let mut naca = Vec::new();
    for slot in &b.order {
        match slot {
            Slot::Namelist(idx) => {
                // SYMFLP and ASYFLP are mutually exclusive; the later one wins.
                namelist::read_namelist(&mut cursor, &mut namelists[*idx])?;
                namelists[*idx].present = true;
                finalize::on_namelist_read(&mut namelists, *idx);
            }
            Slot::Naca => {
                if let Some(card) = cursor.next_card() {
                    naca.push(NacaCard {
                        surface: card.b[5] as char,
                        text: card.text(),
                    });
                }
            }
        }
    }
    let mut case = Case {
        id: b.id,
        controls: b.controls,
        dim,
        namelists,
        naca,
        expr: b.expr,
        diagnostics: b.diagnostics,
        warnings: Vec::new(),
        regimes: Regimes::default(),
    };
    finalize_case(&mut case);
    Ok(case)
}

#[cfg(test)]
mod tests;
