# ModelicaAutomotive traceability

This matrix maps every public executable class to its requirement, governing
reference or implementation basis, and direct validation. Package records,
types, connectors, constants, ordering, and MSL-only dependency metadata are
validated by the structural and common-definition tests.

| Public classes | Requirement | Reference or basis | Direct validation |
| --- | --- | --- | --- |
| `ModelicaAutomotive.Utilities.assertPositive`, `ModelicaAutomotive.Utilities.assertInRange` | Reusable parameter contract helpers | Explicit Modelica assertions | `tests\test_common_definitions.py`, `ModelicaAutomotive.Tests.Common.CommonValidation` |
| `ModelicaAutomotive.Examples.PackageSmoke` | Minimal package and compiler smoke model | Constant signal | `tests\test_compiler_smoke.py` |

The structural test derives public executable classes from the Modelica source
tree and fails if a future class is absent from this matrix.
