# ModelicaMaritime traceability

This matrix maps every public executable class to its requirement, governing
reference or implementation basis, and direct validation. Package records,
types, connectors, and constants are validated by
`tests\test_common_definitions.py`; package ordering and MSL-only dependency
metadata are validated by `tests\test_package_structure.py`.

| Public classes | Requirement | Reference or basis | Direct validation |
| --- | --- | --- | --- |
| `ModelicaMaritime.Utilities.assertPositive`, `ModelicaMaritime.Utilities.assertInRange` | Reusable parameter contract helpers | Explicit Modelica assertions | `tests\test_common_definitions.py`, `ModelicaMaritime.Tests.Common.CommonValidation` |
| `ModelicaMaritime.Examples.PackageSmoke` | Minimal package and compiler smoke model | Constant signal | `tests\test_compiler_smoke.py` |

The structural test derives public executable classes from the Modelica source
tree and fails if a future class is not named in this matrix.
