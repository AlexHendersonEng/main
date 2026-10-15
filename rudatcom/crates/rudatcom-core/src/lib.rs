//! rudatcom-core: Rust rewrite of the USAF Digital DATCOM.
//!
//! Modules are added as the Fortran source (`datcom.f`) is ported; see `docs/architecture.md`.

pub mod compare;

/// Errors returned by the library.
#[derive(Debug, thiserror::Error)]
pub enum Error {
    /// The requested functionality has not been ported yet.
    #[error("not implemented yet")]
    NotImplemented,
}

/// Run a DATCOM input deck and return the `datcom.out` report text.
///
/// Placeholder until the parser and methods are ported; the golden tests call this.
pub fn run_case(_input: &str) -> Result<String, Error> {
    Err(Error::NotImplemented)
}

/// Version of the rudatcom core library.
pub fn version() -> &'static str {
    env!("CARGO_PKG_VERSION")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn version_is_not_empty() {
        assert!(!version().is_empty());
    }
}
