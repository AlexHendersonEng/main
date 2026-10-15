//! Native extension backing the `rudatcom` Python package (built with maturin).

use pyo3::prelude::*;

/// Version of the underlying rudatcom core library.
#[pyfunction]
fn version() -> &'static str {
    rudatcom_core::version()
}

#[pymodule]
fn _rudatcom(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(version, m)?)?;
    Ok(())
}
