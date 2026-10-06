# ModelicaMaritime traceability

This matrix maps every public executable class to its requirement, governing
reference or implementation basis, and direct validation. Package records,
types, connectors, and constants are validated by
`tests\test_common_definitions.py`; package ordering and MSL-only dependency
metadata are validated by `tests\test_package_structure.py`.

| Public classes | Requirement | Reference or basis | Direct validation |
| --- | --- | --- | --- |
| `ModelicaMaritime.Mathematics.skewMatrix`, `ModelicaMaritime.Mathematics.wrapHeading` | Cross-product matrix support and headings wrapped to `[0, 2*pi)` | Standard vector algebra and trigonometric principal-angle wrapping | `tests\test_coordinates.py`, `ModelicaMaritime.Tests.Coordinates.CoordinateValidation` |
| `ModelicaMaritime.Coordinates.rotationBodyToNED321`, `ModelicaMaritime.Coordinates.bodyToNEDVector`, `ModelicaMaritime.Coordinates.nedToBodyVector` | Active 3-2-1 body/NED rotation and inverse vector transformations | Standard roll-pitch-yaw direction cosine matrix | `tests\test_coordinates.py`, `ModelicaMaritime.Tests.Coordinates.CoordinateValidation` |
| `ModelicaMaritime.Coordinates.planarKinematics`, `ModelicaMaritime.Coordinates.bodyRatesToEuler321Rates`, `ModelicaMaritime.Coordinates.rigidBodyKinematics321` | Planar and 6-DoF body-velocity to navigation-state kinematics with explicit Euler singularity handling | Marine-craft NED kinematic transformations | `tests\test_coordinates.py`, `ModelicaMaritime.Tests.Coordinates` |
| `ModelicaMaritime.Coordinates.depthFromPositionNED`, `ModelicaMaritime.Coordinates.altitudeAboveSeafloor` | Stable positive-down depth and positive-up seafloor-altitude conventions | NED geometry | `tests\test_coordinates.py`, `ModelicaMaritime.Tests.Coordinates.CoordinateValidation` |
| `ModelicaMaritime.Environment.Water.densityLinear`, `ModelicaMaritime.Environment.Water.hydrostaticPressure`, `ModelicaMaritime.Environment.Water.Blocks.ConstantWater`, `ModelicaMaritime.Environment.Water.Blocks.LinearWater` | Low-order constant and depth-varying seawater state and pressure models | Linear thermal/haline density approximation and hydrostatic pressure | `tests\test_environment.py`, `ModelicaMaritime.Tests.Environment.EnvironmentValidation` |
| `ModelicaMaritime.Environment.Current.steadyCurrentNED`, `ModelicaMaritime.Environment.Current.linearCurrentProfile`, `ModelicaMaritime.Environment.Current.Blocks.SteadyCurrent`, `ModelicaMaritime.Environment.Current.Blocks.LinearCurrent` | Deterministic steady and depth-varying NED current primitives | Horizontal direction geometry and linear profiles | `tests\test_environment.py`, `ModelicaMaritime.Tests.Environment.EnvironmentValidation` |
| `ModelicaMaritime.Utilities.assertPositive`, `ModelicaMaritime.Utilities.assertInRange` | Reusable parameter contract helpers | Explicit Modelica assertions | `tests\test_common_definitions.py`, `ModelicaMaritime.Tests.Common.CommonValidation` |
| `ModelicaMaritime.Examples.PackageSmoke` | Minimal package and compiler smoke model | Constant signal | `tests\test_compiler_smoke.py` |

The structural test derives public executable classes from the Modelica source
tree and fails if a future class is not named in this matrix.
