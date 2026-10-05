# ModelicaAutomotive traceability

This matrix maps every public executable class to its requirement, governing
reference or implementation basis, and direct validation. Package records,
types, connectors, constants, ordering, and MSL-only dependency metadata are
validated by the structural and common-definition tests.

| Public classes | Requirement | Reference or basis | Direct validation |
| --- | --- | --- | --- |
| `ModelicaAutomotive.Utilities.assertPositive`, `ModelicaAutomotive.Utilities.assertInRange` | Reusable parameter contract helpers | Explicit Modelica assertions | `tests\test_common_definitions.py`, `ModelicaAutomotive.Tests.Common.CommonValidation` |
| `ModelicaAutomotive.Examples.PackageSmoke` | Minimal package and compiler smoke model | Constant signal | `tests\test_compiler_smoke.py` |
| `ModelicaAutomotive.Mathematics.regularizedSign`, `ModelicaAutomotive.Mathematics.wrapAngle`, `ModelicaAutomotive.Mathematics.roadToWorldMatrix`, `ModelicaAutomotive.Mathematics.transformVector`, `ModelicaAutomotive.Mathematics.aggregateForcesMoments` | Solver-robust low-speed behavior, wrapped angles, road-frame transforms, and four-corner load aggregation | Smooth sign approximation, rigid rotations, and force/moment balance | `tests\test_mathematics_road.py`, `ModelicaAutomotive.Tests.MathematicsRoad` |
| `ModelicaAutomotive.Road.roadHeight`, `ModelicaAutomotive.Road.roadNormal`, `ModelicaAutomotive.Road.ConstantRoad`, `ModelicaAutomotive.Road.FourCornerRoad` | Grade, bank, crown, friction, and wheel contact-point road queries | Locally parameterized analytic road surface | `tests\test_mathematics_road.py`, `ModelicaAutomotive.Tests.MathematicsRoad` |
| `ModelicaAutomotive.VehicleDynamics.Longitudinal.Body` | One-dimensional motion with tire, aerodynamic, rolling, and grade forces | Newton's second law and standard road-load equations | `tests\test_longitudinal.py`, `ModelicaAutomotive.Tests.Longitudinal` |
| `ModelicaAutomotive.Examples.LongitudinalDrive` | Bounded acceleration and coastdown composition | Independent numerical integration of the documented road-load equations | `tests\test_examples.py` |

The structural test derives public executable classes from the Modelica source
tree and fails if a future class is absent from this matrix.
