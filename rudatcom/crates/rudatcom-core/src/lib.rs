//! rudatcom-core: Rust rewrite of the USAF Digital DATCOM.
//!
//! Modules are added as the Fortran source (`datcom.f`) is ported; see `docs/architecture.md`.

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
