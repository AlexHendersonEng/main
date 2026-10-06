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
| `ModelicaAutomotive.Tires.Kinematics`, `ModelicaAutomotive.Tires.CombinedSlipLimiter` | Regularized wheel slip and bounded combined tire forces | Standard slip definitions and friction-circle scaling | `tests\test_tires.py`, `ModelicaAutomotive.Tests.Tires` |
| `ModelicaAutomotive.Tires.LinearTire`, `ModelicaAutomotive.Tires.FialaTire`, `ModelicaAutomotive.Tires.MagicFormulaTire` | Linear, brush, and compact empirical pure-slip tire forces with combined-slip limiting | Linear stiffness, Fiala brush equations, and parameterized Magic Formula equations | `tests\test_tires.py`, `ModelicaAutomotive.Tests.Tires` |
| `ModelicaAutomotive.Wheels.RotationalDynamics` | Wheel angular response to hub, brake, and tire torques | Rotational Newton's second law | `tests\test_wheels_brakes.py`, `ModelicaAutomotive.Tests.WheelsBrakes` |
| `ModelicaAutomotive.Brakes.IdealBrake`, `ModelicaAutomotive.Brakes.FirstOrderBrake`, `ModelicaAutomotive.Brakes.FrictionLimitedBrake`, `ModelicaAutomotive.Brakes.MappedBrake` | Static, dynamic, friction-limited, and mapped brake torque | Explicit torque limits, first-order response, and MSL `CombiTable1Ds` | `tests\test_wheels_brakes.py`, `ModelicaAutomotive.Tests.WheelsBrakes` |
| `ModelicaAutomotive.Examples.TireSweep`, `ModelicaAutomotive.Examples.SplitFrictionBraking` | Tire-family comparison and unequal-friction braking composition | Force envelopes and coupled wheel/body dynamics | `tests\test_examples.py` |

The structural test derives public executable classes from the Modelica source
tree and fails if a future class is absent from this matrix.
