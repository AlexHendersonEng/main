"""Study the aerodynamics and flight dynamics of a simple general-aviation aircraft with AeroSandbox."""

from collections.abc import Callable
from pathlib import Path
import aerosandbox as asb
import matplotlib.pyplot as plt
import numpy as np
from scipy.optimize import root

# Save the geometry render beside this script.
OUTPUT_GEOMETRY_PATH = Path(__file__).resolve().parent / "aircraft_demo.png"
# Save the aerodynamic study plots beside this script.
OUTPUT_STUDY_PATH = Path(__file__).resolve().parent / "aircraft_aero_study.png"
# Save the control-surface response plots beside this script.
OUTPUT_CONTROL_PATH = Path(__file__).resolve().parent / "aircraft_control_surfaces.png"
# Save the control-derivative summary plots beside this script.
OUTPUT_CONTROL_DERIVATIVE_PATH = (
    Path(__file__).resolve().parent / "aircraft_control_derivatives.png"
)
# Use a cambered aerofoil for the main wing.
MAIN_AIRFOIL = asb.Airfoil("naca2412")
# Use a symmetric aerofoil for both tail surfaces.
TAIL_AIRFOIL = asb.Airfoil("naca0012")
# Set the cruise conditions for the study.
CRUISE_SPEED = 55.0
CRUISE_ALPHA = 2.0
CRUISE_BETA = 0.0
# Use a one-degree step for the control-derivative finite differences.
CONTROL_DEFLECTION = 1.0
# List the force and moment coefficients reported for every case.
COEFFICIENT_NAMES = ("CL", "CD", "CY", "Cl", "Cm", "Cn")
# Map each differentiation variable to the AeroSandbox derivative suffix.
DERIVATIVE_SUFFIXES = {"alpha": "a", "beta": "b", "p": "p", "q": "q", "r": "r"}
# Set the deflection ranges used for the control-surface response plots.
CONTROL_SWEEP_RANGES = {
    "Flap": np.linspace(-5.0, 20.0, 11),
    "Aileron": np.linspace(-15.0, 15.0, 13),
    "Elevator": np.linspace(-15.0, 15.0, 13),
    "Rudder": np.linspace(-20.0, 20.0, 13),
}
# Give each coefficient a consistent colour across the bar charts.
COEFFICIENT_COLOURS = {
    "CL": "tab:blue",
    "CD": "tab:red",
    "CY": "tab:orange",
    "Cl": "tab:green",
    "Cm": "tab:purple",
    "Cn": "tab:brown",
}
# Set the gravitational acceleration used by the trajectory model.
GRAVITY = 9.81
# Set the aircraft mass used for the six-degree-of-freedom simulations.
TRAJECTORY_MASS = 1400.0
# Start the trajectories at a representative cruising altitude.
TRAJECTORY_ALTITUDE = 500.0
# Integrate each example trajectory for eight seconds.
TRAJECTORY_DURATION = 8.0
# Sample the trajectory states every twentieth of a second.
TRAJECTORY_SAMPLE_INTERVAL = 0.02
# Limit the adaptive integrator to a tenth-of-a-second step.
TRAJECTORY_MAX_STEP = 0.1
# Set the start, switch and end times of every control doublet.
TRAJECTORY_PULSE_TIMES = (1.0, 1.5, 2.0)
# Describe the control doublet used by each example trajectory.
TRAJECTORY_CASES = {
    "elevator": {"control": "Elevator", "amplitude": 4.0},
    "aileron": {"control": "Aileron", "amplitude": 4.0},
    "rudder": {"control": "Rudder", "amplitude": 3.0},
}
# List the twelve state variables in integration order.
STATE_NAMES = (
    "x_e",
    "y_e",
    "z_e",
    "u_b",
    "v_b",
    "w_b",
    "phi",
    "theta",
    "psi",
    "p",
    "q",
    "r",
)
# List the controls recorded in every trajectory history.
RECORDED_CONTROLS = ("Elevator", "Aileron", "Rudder")
# Save each trajectory plot beside this script.
OUTPUT_TRAJECTORY_PATHS = {
    name: Path(__file__).resolve().parent / f"aircraft_trajectory_{name}.png"
    for name in TRAJECTORY_CASES
}
# Save the combined three-dimensional trajectory plot beside this script.
OUTPUT_TRAJECTORY_3D_PATH = (
    Path(__file__).resolve().parent / "aircraft_trajectory_3d.png"
)


def build_fuselage() -> asb.Fuselage:
    """Build a tapered fuselage from nose to tail."""
    # Define elliptic cross-sections from nose to tail.
    return asb.Fuselage(
        name="Fuselage",
        xsecs=[
            # Rounded nose tip.
            asb.FuselageXSec(xyz_c=[0.00, 0, 0.00], width=0.08, height=0.08),
            # Engine cowling.
            asb.FuselageXSec(xyz_c=[0.45, 0, 0.00], width=0.65, height=0.62),
            # Cabin front.
            asb.FuselageXSec(xyz_c=[1.30, 0, 0.00], width=1.15, height=1.22),
            # Widest cabin section.
            asb.FuselageXSec(xyz_c=[2.40, 0, 0.03], width=1.30, height=1.35),
            # Cabin rear.
            asb.FuselageXSec(xyz_c=[3.90, 0, 0.02], width=1.20, height=1.30),
            # Tail boom start.
            asb.FuselageXSec(xyz_c=[5.40, 0, 0.08], width=0.80, height=0.90),
            # Tail boom end.
            asb.FuselageXSec(xyz_c=[6.60, 0, 0.12], width=0.45, height=0.58),
            # Tail cone tip.
            asb.FuselageXSec(xyz_c=[7.40, 0, 0.15], width=0.15, height=0.30),
        ],
    )


def build_main_wing() -> asb.Wing:
    """Build a tapered, high-mounted main wing with dihedral, washout and controls."""
    # Mirror the half-wing about the symmetry plane.
    return asb.Wing(
        name="Main wing",
        symmetric=True,
        xsecs=[
            # Root section, mounted above the cabin and carrying the flaps.
            asb.WingXSec(
                xyz_le=[1.90, 0.00, 0.95],
                chord=1.60,
                twist=2.0,
                airfoil=MAIN_AIRFOIL,
                control_surfaces=[
                    asb.ControlSurface(name="Flap", symmetric=True, hinge_point=0.75),
                ],
            ),
            # Mid section, carrying the ailerons outboard of the flaps.
            asb.WingXSec(
                xyz_le=[2.175, 2.75, 1.045],
                chord=1.30,
                twist=1.0,
                airfoil=MAIN_AIRFOIL,
                control_surfaces=[
                    asb.ControlSurface(
                        name="Aileron", symmetric=False, hinge_point=0.80
                    ),
                ],
            ),
            # Tip section, swept back and raised by dihedral.
            asb.WingXSec(
                xyz_le=[2.45, 5.50, 1.14],
                chord=1.00,
                twist=0.0,
                airfoil=MAIN_AIRFOIL,
            ),
        ],
    )


def build_horizontal_tail() -> asb.Wing:
    """Build the horizontal stabiliser with an elevator."""
    # Mirror the half-tail about the symmetry plane.
    return asb.Wing(
        name="Horizontal stabiliser",
        symmetric=True,
        xsecs=[
            # Root section, blended into the tail cone and carrying the elevator.
            asb.WingXSec(
                xyz_le=[5.95, 0.00, 0.35],
                chord=0.85,
                airfoil=TAIL_AIRFOIL,
                control_surfaces=[
                    asb.ControlSurface(
                        name="Elevator", symmetric=True, hinge_point=0.80
                    ),
                ],
            ),
            # Tip section, slightly swept and raised.
            asb.WingXSec(
                xyz_le=[6.30, 1.80, 0.44],
                chord=0.55,
                airfoil=TAIL_AIRFOIL,
            ),
        ],
    )


def build_vertical_tail() -> asb.Wing:
    """Build the vertical stabiliser with a rudder."""
    # Define root and tip sections; the span runs upwards.
    return asb.Wing(
        name="Vertical stabiliser",
        xsecs=[
            # Root section, blended into the tail cone and carrying the rudder.
            asb.WingXSec(
                xyz_le=[5.90, 0, 0.30],
                chord=1.05,
                airfoil=TAIL_AIRFOIL,
                control_surfaces=[
                    asb.ControlSurface(name="Rudder", symmetric=True, hinge_point=0.80),
                ],
            ),
            # Tip section, swept back.
            asb.WingXSec(
                xyz_le=[6.55, 0, 1.55],
                chord=0.60,
                airfoil=TAIL_AIRFOIL,
            ),
        ],
    )


def build_aircraft() -> asb.Airplane:
    """Assemble the fuselage, lifting surfaces, controls and propeller into one aircraft."""
    # Set the moment reference point near the wing quarter-chord.
    return asb.Airplane(
        name="Simple general-aviation aircraft",
        xyz_ref=[2.30, 0, 0],
        # Add the single fuselage.
        fuselages=[build_fuselage()],
        # Add the main wing and both tail surfaces.
        wings=[
            build_main_wing(),
            build_horizontal_tail(),
            build_vertical_tail(),
        ],
        # Add a propeller as a thin actuator disk at the nose.
        propulsors=[
            asb.Propulsor(
                name="Propeller",
                xyz_c=[0.42, 0, 0],
                xyz_normal=[-1, 0, 0],
                radius=0.78,
                length=0.0,
            ),
        ],
    )


def to_float(value) -> float:
    """Convert a size-one AeroSandbox result to a float."""
    return float(np.asarray(value).reshape(-1)[0])


def run_point_analysis(aircraft: asb.Airplane, op_point: asb.OperatingPoint) -> dict:
    """Run a point analysis with forces, moments and stability derivatives."""
    return asb.AeroBuildup(
        airplane=aircraft, op_point=op_point
    ).run_with_stability_derivatives()


def run_sweep(
    aircraft: asb.Airplane,
    atmosphere: asb.Atmosphere,
    alpha: float | np.ndarray,
    beta: float | np.ndarray,
) -> dict:
    """Run a vectorised AeroBuildup sweep at the cruise speed."""
    # Build the vectorised operating point for the sweep.
    op_point = asb.OperatingPoint(
        atmosphere=atmosphere,
        velocity=CRUISE_SPEED,
        alpha=alpha,
        beta=beta,
    )
    # Return the complete force and moment coefficient set.
    return asb.AeroBuildup(airplane=aircraft, op_point=op_point).run()


def run_control_sweep(
    aircraft: asb.Airplane,
    atmosphere: asb.Atmosphere,
    control_name: str,
    deflections: np.ndarray,
) -> dict[str, np.ndarray]:
    """Run a control-surface deflection sweep at the cruise point."""
    # Build the neutral cruise operating point.
    op_point = asb.OperatingPoint(
        atmosphere=atmosphere,
        velocity=CRUISE_SPEED,
        alpha=CRUISE_ALPHA,
        beta=CRUISE_BETA,
    )
    # Store one list per coefficient while stepping through the deflections.
    results = {coefficient: [] for coefficient in COEFFICIENT_NAMES}
    for deflection in deflections:
        # Apply the deflection to the requested control surface.
        deflected_aircraft = aircraft.with_control_deflections(
            {control_name: float(deflection)}
        )
        # Run the point analysis for this deflection.
        point_results = asb.AeroBuildup(
            airplane=deflected_aircraft,
            op_point=op_point,
        ).run()
        # Record every coefficient at this deflection.
        for coefficient in COEFFICIENT_NAMES:
            results[coefficient].append(to_float(point_results[coefficient]))
    # Convert the accumulated lists to arrays for plotting.
    return {name: np.asarray(values) for name, values in results.items()}


def compute_control_derivatives(
    aircraft: asb.Airplane,
    atmosphere: asb.Atmosphere,
) -> dict[str, dict[str, float]]:
    """Estimate control derivatives with central finite differences."""
    derivatives = {}
    # Build the neutral cruise operating point.
    op_point = asb.OperatingPoint(
        atmosphere=atmosphere,
        velocity=CRUISE_SPEED,
        alpha=CRUISE_ALPHA,
        beta=CRUISE_BETA,
    )
    # Collect the names of every control surface on the aircraft.
    control_names = []
    for wing in aircraft.wings:
        control_names.extend(wing.get_control_surface_names())
    # Differentiate each control surface while holding the others neutral.
    for name in control_names:
        # Run the positive and negative perturbations.
        positive = asb.AeroBuildup(
            airplane=aircraft.with_control_deflections({name: CONTROL_DEFLECTION}),
            op_point=op_point,
        ).run()
        negative = asb.AeroBuildup(
            airplane=aircraft.with_control_deflections({name: -CONTROL_DEFLECTION}),
            op_point=op_point,
        ).run()
        # Convert both perturbations to a per-radian derivative.
        denominator = np.radians(2 * CONTROL_DEFLECTION)
        derivatives[name] = {
            coefficient: (
                to_float(positive[coefficient]) - to_float(negative[coefficient])
            )
            / denominator
            for coefficient in COEFFICIENT_NAMES
        }
    return derivatives


def control_surface_areas(aircraft: asb.Airplane) -> dict[str, float]:
    """Return the planform area of every control surface."""
    areas = {}
    # Read the area of each named control surface from its parent wing.
    for wing in aircraft.wings:
        for name in wing.get_control_surface_names():
            areas[name] = float(wing.control_surface_area(by_name=name))
    return areas


def print_reference_data(aircraft: asb.Airplane) -> None:
    """Print the reference geometry and flight condition."""
    print()
    print("Reference data")
    print("--------------")
    # Print the geometry reference values used to non-dimensionalise the results.
    print(f"Aircraft: {aircraft.name}")
    print(f"Reference area: {aircraft.s_ref:.3f} m^2")
    print(f"Reference chord: {aircraft.c_ref:.3f} m")
    print(f"Reference span: {aircraft.b_ref:.3f} m")
    print(f"Reference point: {np.asarray(aircraft.xyz_ref)} m")
    # Print the cruise condition used for the point study.
    print(
        f"Cruise condition: V = {CRUISE_SPEED:.1f} m/s, "
        f"alpha = {CRUISE_ALPHA:.1f} deg, beta = {CRUISE_BETA:.1f} deg, sea level"
    )
    # Print the control surfaces and their planform areas.
    print()
    print("Control surfaces")
    print("----------------")
    for name, area in control_surface_areas(aircraft).items():
        print(f"{name:<10s} {area:7.3f} m^2")


def print_force_and_moment_coefficients(results: dict) -> None:
    """Print the force and moment coefficients at the cruise point."""
    print()
    print("Force and moment coefficients at cruise")
    print("---------------------------------------")
    # Print the force coefficients.
    print(
        f"CL = {to_float(results['CL']): .5f}    "
        f"CD = {to_float(results['CD']): .5f}    "
        f"CY = {to_float(results['CY']): .5f}"
    )
    # Print the moment coefficients.
    print(
        f"Cl = {to_float(results['Cl']): .5f}    "
        f"Cm = {to_float(results['Cm']): .5f}    "
        f"Cn = {to_float(results['Cn']): .5f}"
    )
    # Print the dimensional forces and the lift-to-drag ratio.
    lift = to_float(results["L"])
    drag = to_float(results["D"])
    print(f"Lift = {lift: .1f} N    Drag = {drag: .1f} N    L/D = {lift / drag:.2f}")
    # Report the equivalent mass supported in level flight.
    print(f"Equivalent mass at 1 g = {lift / 9.81:.0f} kg")


def print_stability_derivatives(results: dict) -> None:
    """Print the full matrix of stability derivatives."""
    print()
    print("Stability derivatives")
    print("---------------------")
    # Print the header for the six-by-five derivative matrix.
    print(f"{'':<8s}" + "".join(f"{name:>11s}" for name in DERIVATIVE_SUFFIXES))
    # Print every derivative coefficient for every differentiation variable.
    for coefficient in COEFFICIENT_NAMES:
        values = [
            to_float(results[f"{coefficient}{suffix}"])
            for suffix in DERIVATIVE_SUFFIXES.values()
        ]
        print(f"{coefficient:<8s}" + "".join(f"{value:11.4f}" for value in values))
    # Print the resulting neutral points.
    print(f"Neutral point x_np = {to_float(results['x_np']):.3f} m")
    print(
        f"Lateral neutral point x_np_lateral = {to_float(results['x_np_lateral']):.3f} m"
    )
    # Clarify the rate normalisation used by AeroSandbox.
    print(
        "Rate columns use p*b/(2V), q*c/(2V) and r*b/(2V); other columns are per radian."
    )


def print_control_derivatives(
    derivatives: dict[str, dict[str, float]],
    areas: dict[str, float],
) -> None:
    """Print the control-surface derivative matrix."""
    print()
    print("Control surface derivatives")
    print("---------------------------")
    # Print the header for the six-by-four control derivative matrix.
    print(
        f"{'Control':<10s}{'Area':>8s}"
        + "".join(f"{name:>10s}" for name in COEFFICIENT_NAMES)
    )
    # Print every coefficient derivative for every control surface.
    for name, values in derivatives.items():
        print(
            f"{name:<10s}{areas[name]:8.3f}"
            + "".join(
                f"{values[coefficient]:10.4f}" for coefficient in COEFFICIENT_NAMES
            )
        )
    # State the differentiation units.
    print("Control derivatives are per radian of trailing-edge-down deflection.")


def build_mass_properties() -> asb.MassProperties:
    """Assemble the component masses into a combined mass-properties object."""
    # List the mass, centre of gravity and inertia of every major component.
    components = [
        # Fuselage structure, systems and cabin.
        asb.MassProperties(
            mass=560.0,
            x_cg=2.80,
            y_cg=0.0,
            z_cg=0.05,
            Ixx=120.0,
            Iyy=2557.0,
            Izz=2000.0,
        ),
        # Main wing, including the flaps and ailerons.
        asb.MassProperties(
            mass=300.0,
            x_cg=2.35,
            y_cg=0.0,
            z_cg=1.05,
            Ixx=3025.0,
            Iyy=30.0,
            Izz=3025.0,
        ),
        # Horizontal stabiliser and elevator.
        asb.MassProperties(
            mass=45.0,
            x_cg=6.15,
            y_cg=0.0,
            z_cg=0.40,
            Ixx=49.0,
            Iyy=3.0,
            Izz=49.0,
        ),
        # Vertical stabiliser and rudder.
        asb.MassProperties(
            mass=35.0,
            x_cg=6.20,
            y_cg=0.0,
            z_cg=0.90,
            Ixx=5.0,
            Iyy=5.0,
            Izz=3.0,
        ),
        # Engine, propeller and accessories.
        asb.MassProperties(
            mass=280.0,
            x_cg=0.60,
            y_cg=0.0,
            z_cg=0.0,
            Ixx=50.0,
            Iyy=70.0,
            Izz=70.0,
        ),
        # Fuel and payload.
        asb.MassProperties(
            mass=180.0,
            x_cg=2.40,
            y_cg=0.0,
            z_cg=0.15,
            Ixx=40.0,
            Iyy=60.0,
            Izz=60.0,
        ),
    ]
    # Start from the first component and add the remaining components.
    total = components[0]
    for component in components[1:]:
        # Combine the component mass properties about their common centre of gravity.
        total = total + component

    # Confirm that the component masses match the intended aircraft mass.
    if not np.isclose(total.mass, TRAJECTORY_MASS):
        raise ValueError(
            f"Component masses total {total.mass:.1f} kg, "
            f"expected {TRAJECTORY_MASS:.1f} kg"
        )

    # Return the combined mass properties.
    return total


def reference_offset_body(
    aircraft: asb.Airplane,
    mass_props: asb.MassProperties,
) -> np.ndarray:
    """Return the aerodynamic reference point relative to the centre of gravity in body axes."""
    # Read the aerodynamic moment reference point in geometry axes.
    reference = np.asarray(aircraft.xyz_ref, dtype=float)
    # Read the centre of gravity in geometry axes.
    centre = np.asarray(
        [mass_props.x_cg, mass_props.y_cg, mass_props.z_cg],
        dtype=float,
    )
    # Form the offset in geometry axes.
    offset = reference - centre

    # Return the offset with the geometry-to-body sign changes.
    return np.array([-offset[0], offset[1], -offset[2]])


def body_aero_loads(
    results: dict,
    offset_body: np.ndarray,
) -> tuple[np.ndarray, np.ndarray]:
    """Return the aerodynamic force and centre-of-gravity moment in body axes."""
    # Extract the body-axis force components from the AeroBuildup result.
    force = np.array([to_float(results["F_b"][axis]) for axis in range(3)])
    # Extract the body-axis moments about the aerodynamic reference point.
    moment = np.array([to_float(results["M_b"][axis]) for axis in range(3)])
    # Transfer the moment to the centre of gravity.
    moment = moment + np.cross(offset_body, force)

    # Return the force and the centre-of-gravity moment.
    return force, moment


def trim_aircraft(
    aircraft: asb.Airplane,
    atmosphere: asb.Atmosphere,
    mass_props: asb.MassProperties,
) -> tuple[float, float]:
    """Find the angle of attack and elevator deflection that trim the aircraft."""
    # Locate the aerodynamic reference point relative to the centre of gravity.
    offset_body = reference_offset_body(aircraft, mass_props)
    # Read the reference geometry needed to non-dimensionalise the moment.
    area = aircraft.s_ref
    chord = aircraft.c_ref

    def residual(values: np.ndarray) -> list[float]:
        """Return the body-z force error and the centre-of-gravity pitching moment."""
        # Unpack the angle of attack and elevator deflection in degrees.
        alpha, elevator = values
        # Apply the elevator deflection to the neutral aircraft.
        deflected = aircraft.with_control_deflections({"Elevator": elevator})
        # Build the operating point at the cruise speed and zero sideslip.
        op_point = asb.OperatingPoint(
            atmosphere=atmosphere,
            velocity=CRUISE_SPEED,
            alpha=alpha,
            beta=0.0,
        )
        # Run the aerodynamic analysis at this trim guess.
        results = asb.AeroBuildup(deflected, op_point).run()
        # Extract the centre-of-gravity moment.
        _, moment = body_aero_loads(results, offset_body)
        # Resolve the lift and drag into the body-z direction.
        alpha_rad = np.radians(alpha)
        vertical = to_float(results["L"]) * np.cos(alpha_rad) + to_float(
            results["D"]
        ) * np.sin(alpha_rad)
        # Non-dimensionalise the body-z force error and pitching moment.
        force_error = (vertical - mass_props.mass * GRAVITY * np.cos(alpha_rad)) / (
            mass_props.mass * GRAVITY
        )
        moment_coefficient = moment[1] / (op_point.dynamic_pressure() * area * chord)

        # Return both trim residuals.
        return [force_error, moment_coefficient]

    # Solve the two trim equations from a level-flight initial guess.
    solution = root(residual, [CRUISE_ALPHA, -6.0], method="hybr", tol=1e-8)
    # Stop if the trim solve did not converge.
    if not solution.success:
        raise RuntimeError(f"Trim solution failed: {solution.message}")

    # Return the trimmed angle of attack and elevator deflection in degrees.
    alpha, elevator = (float(value) for value in solution.x)

    return alpha, elevator


def trim_thrust(
    aircraft: asb.Airplane,
    atmosphere: asb.Atmosphere,
    mass_props: asb.MassProperties,
    alpha: float,
    elevator: float,
) -> float:
    """Return the constant thrust that balances body-axis forces at trim."""
    # Apply the trim elevator deflection.
    deflected = aircraft.with_control_deflections({"Elevator": elevator})
    # Build the trim operating point.
    op_point = asb.OperatingPoint(
        atmosphere=atmosphere,
        velocity=CRUISE_SPEED,
        alpha=alpha,
        beta=0.0,
    )
    # Run the aerodynamic analysis at trim.
    results = asb.AeroBuildup(deflected, op_point).run()
    # Balance the body-axis aerodynamic force with the weight component and thrust.
    thrust = mass_props.mass * GRAVITY * np.sin(np.radians(alpha)) - to_float(
        results["F_b"][0]
    )

    # Return the constant thrust.
    return thrust


def build_initial_state(alpha: float) -> np.ndarray:
    """Return the level-flight state vector at the trim angle of attack."""
    # Convert the trim angle of attack to radians.
    alpha_rad = np.radians(alpha)
    # Build the position, body velocity, attitude and rate states.
    initial_state = np.array(
        [
            0.0,
            0.0,
            -TRAJECTORY_ALTITUDE,
            CRUISE_SPEED * np.cos(alpha_rad),
            0.0,
            CRUISE_SPEED * np.sin(alpha_rad),
            0.0,
            alpha_rad,
            0.0,
            0.0,
            0.0,
            0.0,
        ]
    )

    # Return the twelve-state vector.
    return initial_state


def build_control_schedule(
    trim_elevator: float,
    control_name: str,
    amplitude: float,
) -> Callable[[float], dict[str, float]]:
    """Return a doublet schedule for one control surface relative to trim."""
    # Read the doublet timing.
    start, switch, end = TRAJECTORY_PULSE_TIMES

    def schedule(time_value: float) -> dict[str, float]:
        """Return the control deflections at one time."""
        # Start from the trim elevator and neutral lateral controls.
        deflections = {
            "Elevator": trim_elevator,
            "Aileron": 0.0,
            "Rudder": 0.0,
        }
        # Select the sign of the doublet at this time.
        sign = 0.0
        if start <= time_value < switch:
            sign = 1.0
        elif switch <= time_value < end:
            sign = -1.0
        # Add the doublet to the active control surface.
        deflections[control_name] += sign * amplitude

        # Return the complete deflection set.
        return deflections

    # Return the time-dependent schedule.
    return schedule


def simulate_trajectory(
    aircraft: asb.Airplane,
    mass_props: asb.MassProperties,
    atmosphere: asb.Atmosphere,
    initial_state: np.ndarray,
    control_schedule: Callable[[float], dict[str, float]],
    thrust: float,
) -> tuple[np.ndarray, np.ndarray]:
    """Integrate the six-degree-of-freedom equations of motion for one trajectory."""
    # Locate the aerodynamic reference point relative to the centre of gravity.
    offset_body = reference_offset_body(aircraft, mass_props)

    def equations_of_motion(time_value: float, state: np.ndarray) -> np.ndarray:
        """Return the twelve state derivatives at one integration step."""
        # Create a dynamics object for the current state.
        dynamics = asb.DynamicsRigidBody3DBodyEuler(mass_props=mass_props)
        # Unpack the twelve state variables.
        (
            dynamics.x_e,
            dynamics.y_e,
            dynamics.z_e,
            dynamics.u_b,
            dynamics.v_b,
            dynamics.w_b,
            dynamics.phi,
            dynamics.theta,
            dynamics.psi,
            dynamics.p,
            dynamics.q,
            dynamics.r,
        ) = state
        # Guard against a stationary operating point.
        speed = max(float(dynamics.speed), 5.0)
        # Build the operating point from the current body-axis state.
        op_point = asb.OperatingPoint(
            atmosphere=atmosphere,
            velocity=speed,
            alpha=float(dynamics.alpha),
            beta=float(dynamics.beta),
            p=float(dynamics.p),
            q=float(dynamics.q),
            r=float(dynamics.r),
        )
        # Apply the scheduled control deflections.
        configured = aircraft.with_control_deflections(control_schedule(time_value))
        # Run the aerodynamic analysis at the current state.
        results = asb.AeroBuildup(configured, op_point).run()
        # Extract the body-axis force and centre-of-gravity moment.
        force, moment = body_aero_loads(results, offset_body)
        # Add the aerodynamic force and moment.
        dynamics.add_force(*force, axes="body")
        dynamics.add_moment(*moment, axes="body")
        # Add the weight in earth axes.
        dynamics.add_force(
            0.0,
            0.0,
            mass_props.mass * GRAVITY,
            axes="earth",
        )
        # Add the constant thrust along the body x-axis.
        dynamics.add_force(thrust, 0.0, 0.0, axes="body")
        # Compute the state derivatives.
        derivatives = dynamics.state_derivatives()

        # Return the derivatives in integration order.
        return np.array([derivatives[name] for name in STATE_NAMES])

    # Build the regular sample times for the trajectory.
    sample_times = np.arange(
        0.0,
        TRAJECTORY_DURATION + TRAJECTORY_SAMPLE_INTERVAL,
        TRAJECTORY_SAMPLE_INTERVAL,
    )
    # Integrate the equations of motion.
    solution = asb.numpy.integrate.solve_ivp(
        equations_of_motion,
        (0.0, TRAJECTORY_DURATION),
        initial_state,
        method="RK45",
        t_eval=sample_times,
        rtol=1e-4,
        atol=1e-6,
        max_step=TRAJECTORY_MAX_STEP,
    )
    # Stop if the integrator did not converge.
    if not solution.success:
        raise RuntimeError(f"Trajectory integration failed: {solution.message}")

    # Return the sample times and states.
    return solution.t, solution.y


def print_trajectory_setup(
    mass_props: asb.MassProperties,
    alpha: float,
    elevator: float,
    thrust: float,
) -> None:
    """Print the mass properties and trim condition used by the trajectories."""
    print()
    print("Six-degree-of-freedom trajectory setup")
    print("--------------------------------------")
    # Print the mass properties.
    print(f"Mass: {mass_props.mass:.1f} kg")
    print(
        "Centre of gravity: "
        f"[{mass_props.x_cg:.3f}, {mass_props.y_cg:.3f}, {mass_props.z_cg:.3f}] m"
    )
    print(
        f"Inertia: Ixx = {mass_props.Ixx:.1f}, "
        f"Iyy = {mass_props.Iyy:.1f}, "
        f"Izz = {mass_props.Izz:.1f} kg m^2"
    )
    # Print the trim condition and thrust.
    print(f"Trim angle of attack: {alpha:.3f} deg")
    print(f"Trim elevator deflection: {elevator:.3f} deg")
    print(f"Trim thrust: {thrust:.1f} N")
    # Print the trajectory settings.
    print(f"Constant-atmosphere altitude: {TRAJECTORY_ALTITUDE:.1f} m")
    print(f"Simulation duration: {TRAJECTORY_DURATION:.1f} s")


def print_trajectory_summary(name: str, states: np.ndarray) -> None:
    """Print the peak response of one trajectory."""
    # Convert the attitude and rate states to degrees.
    phi = np.degrees(states[6])
    theta = np.degrees(states[7])
    psi = np.degrees(states[8])
    p = np.degrees(states[9])
    q = np.degrees(states[10])
    r = np.degrees(states[11])
    # Compute the altitude and airspeed histories.
    altitude = -states[2]
    speed = np.sqrt(states[3] ** 2 + states[4] ** 2 + states[5] ** 2)
    # Print the response ranges.
    print(f"{name.capitalize()} doublet:")
    print(f"  Bank range: {phi.min(): .2f} to {phi.max(): .2f} deg")
    print(f"  Pitch range: {theta.min(): .2f} to {theta.max(): .2f} deg")
    print(f"  Yaw range: {psi.min(): .2f} to {psi.max(): .2f} deg")
    print(f"  Roll rate range: {p.min(): .2f} to {p.max(): .2f} deg/s")
    print(f"  Pitch rate range: {q.min(): .2f} to {q.max(): .2f} deg/s")
    print(f"  Yaw rate range: {r.min(): .2f} to {r.max(): .2f} deg/s")
    print(f"  Altitude range: {altitude.min(): .2f} to {altitude.max(): .2f} m")
    print(f"  Airspeed range: {speed.min(): .2f} to {speed.max(): .2f} m/s")


def plot_aircraft(aircraft: asb.Airplane) -> None:
    """Draw the aircraft in an isometric view and save the figure."""
    # Draw the complete aircraft with AeroSandbox, deferring display.
    aircraft.draw(
        backend="matplotlib",
        use_preset_view_angle="right_isometric",
        show=False,
    )
    # Label the view with the aircraft name.
    figure = plt.gcf()
    figure.axes[0].set_title(aircraft.name)
    # Save a high-resolution render.
    figure.savefig(OUTPUT_GEOMETRY_PATH, dpi=200, bbox_inches="tight")
    # Report the output location.
    print(f"Saved aircraft render to: {OUTPUT_GEOMETRY_PATH}")


def plot_aero_study(
    alpha_values: np.ndarray,
    alpha_results: dict,
    beta_values: np.ndarray,
    beta_results: dict,
) -> None:
    """Plot the longitudinal and lateral-directional study results."""
    # Create a three-by-two panel of coefficient plots.
    figure, axes = plt.subplots(3, 2, figsize=(12, 11))
    # Plot lift against angle of attack.
    axes[0, 0].plot(alpha_values, alpha_results["CL"], color="tab:blue")
    axes[0, 0].scatter(
        CRUISE_ALPHA,
        np.interp(CRUISE_ALPHA, alpha_values, alpha_results["CL"]),
        color="black",
        zorder=5,
        label="Cruise",
    )
    axes[0, 0].axvline(CRUISE_ALPHA, color="grey", linestyle="--", linewidth=1)
    axes[0, 0].set_xlabel("alpha [deg]")
    axes[0, 0].set_ylabel("CL [-]")
    axes[0, 0].set_title("Lift curve")
    axes[0, 0].legend()
    # Plot the drag polar.
    axes[0, 1].plot(alpha_results["CD"], alpha_results["CL"], color="tab:red")
    axes[0, 1].scatter(
        np.interp(CRUISE_ALPHA, alpha_values, alpha_results["CD"]),
        np.interp(CRUISE_ALPHA, alpha_values, alpha_results["CL"]),
        color="black",
        zorder=5,
        label="Cruise",
    )
    axes[0, 1].set_xlabel("CD [-]")
    axes[0, 1].set_ylabel("CL [-]")
    axes[0, 1].set_title("Drag polar")
    axes[0, 1].legend()
    # Plot the pitching moment against angle of attack.
    axes[1, 0].plot(alpha_values, alpha_results["Cm"], color="tab:green")
    axes[1, 0].scatter(
        CRUISE_ALPHA,
        np.interp(CRUISE_ALPHA, alpha_values, alpha_results["Cm"]),
        color="black",
        zorder=5,
        label="Cruise",
    )
    axes[1, 0].axvline(CRUISE_ALPHA, color="grey", linestyle="--", linewidth=1)
    axes[1, 0].axhline(0, color="black", linewidth=0.8)
    axes[1, 0].set_xlabel("alpha [deg]")
    axes[1, 0].set_ylabel("Cm [-]")
    axes[1, 0].set_title("Pitching moment")
    axes[1, 0].legend()
    # Plot the lift-to-drag ratio against angle of attack.
    axes[1, 1].plot(
        alpha_values, alpha_results["CL"] / alpha_results["CD"], color="tab:purple"
    )
    axes[1, 1].axvline(CRUISE_ALPHA, color="grey", linestyle="--", linewidth=1)
    axes[1, 1].set_xlabel("alpha [deg]")
    axes[1, 1].set_ylabel("L/D [-]")
    axes[1, 1].set_title("Lift-to-drag ratio")
    # Plot the pitching-moment trim curve against lift coefficient.
    axes[2, 0].plot(alpha_results["CL"], alpha_results["Cm"], color="tab:orange")
    axes[2, 0].scatter(
        np.interp(CRUISE_ALPHA, alpha_values, alpha_results["CL"]),
        np.interp(CRUISE_ALPHA, alpha_values, alpha_results["Cm"]),
        color="black",
        zorder=5,
        label="Cruise",
    )
    axes[2, 0].axhline(0, color="black", linewidth=0.8)
    axes[2, 0].set_xlabel("CL [-]")
    axes[2, 0].set_ylabel("Cm [-]")
    axes[2, 0].set_title("Trim curve")
    axes[2, 0].legend()
    # Plot the lateral-directional coefficients against sideslip.
    for coefficient in ("CY", "Cl", "Cn"):
        axes[2, 1].plot(
            beta_values,
            beta_results[coefficient],
            label=coefficient,
        )
    axes[2, 1].axvline(CRUISE_BETA, color="grey", linestyle="--", linewidth=1)
    axes[2, 1].axhline(0, color="black", linewidth=0.8)
    axes[2, 1].set_xlabel("beta [deg]")
    axes[2, 1].set_ylabel("Coefficient [-]")
    axes[2, 1].set_title("Lateral-directional coefficients")
    axes[2, 1].legend()
    # Add a shared title and tidy the panel spacing.
    figure.suptitle("AeroBuildup aerodynamic study")
    figure.tight_layout(rect=(0, 0, 1, 0.97))
    # Save a high-resolution copy of the study plots.
    figure.savefig(OUTPUT_STUDY_PATH, dpi=200, bbox_inches="tight")
    # Report the output location.
    print(f"Saved aerodynamic study plots to: {OUTPUT_STUDY_PATH}")


def plot_control_surface_response(
    deflections: dict[str, np.ndarray],
    results: dict[str, dict[str, np.ndarray]],
) -> None:
    """Plot the response of each control surface to its deflection."""
    # Create a two-by-two panel of control-surface response plots.
    figure, axes = plt.subplots(2, 2, figsize=(12, 9))
    # Plot the flap response, which primarily changes lift and pitching moment.
    plot_double_axis(
        axes[0, 0],
        deflections["Flap"],
        results["Flap"],
        "Flap deflection [deg]",
        "CL",
        "Cm",
        "Flap: lift and pitching moment",
    )
    # Plot the aileron response, which primarily changes roll and yaw.
    plot_double_axis(
        axes[0, 1],
        deflections["Aileron"],
        results["Aileron"],
        "Aileron deflection [deg]",
        "Cl",
        "Cn",
        "Aileron: roll and adverse yaw",
    )
    # Plot the elevator response, which primarily changes lift and pitching moment.
    plot_double_axis(
        axes[1, 0],
        deflections["Elevator"],
        results["Elevator"],
        "Elevator deflection [deg]",
        "CL",
        "Cm",
        "Elevator: lift and pitching moment",
    )
    # Plot the rudder response, which primarily changes yaw and side force.
    plot_double_axis(
        axes[1, 1],
        deflections["Rudder"],
        results["Rudder"],
        "Rudder deflection [deg]",
        "Cn",
        "CY",
        "Rudder: yaw and side force",
    )
    # Add a shared title and tidy the panel spacing.
    figure.suptitle("Control surface effectiveness")
    figure.tight_layout(rect=(0, 0, 1, 0.97))
    # Save a high-resolution copy of the control-surface plots.
    figure.savefig(OUTPUT_CONTROL_PATH, dpi=200, bbox_inches="tight")
    # Report the output location.
    print(f"Saved control-surface plots to: {OUTPUT_CONTROL_PATH}")


def plot_double_axis(
    axis,
    x_values: np.ndarray,
    results: dict[str, np.ndarray],
    x_label: str,
    left_name: str,
    right_name: str,
    title: str,
) -> None:
    """Plot two coefficients against a control deflection using two axes."""
    # Plot the primary coefficient on the left axis.
    left_line = axis.plot(
        x_values, results[left_name], color="tab:blue", label=left_name
    )
    axis.set_xlabel(x_label)
    axis.set_ylabel(f"{left_name} [-]", color="tab:blue")
    axis.tick_params(axis="y", labelcolor="tab:blue")
    # Mark the neutral deflection and add a light grid.
    axis.axvline(0, color="grey", linestyle="--", linewidth=1)
    axis.grid(True, alpha=0.3)
    # Create the right axis for the coupled coefficient.
    twin = axis.twinx()
    right_line = twin.plot(
        x_values, results[right_name], color="tab:red", label=right_name
    )
    twin.set_ylabel(f"{right_name} [-]", color="tab:red")
    twin.tick_params(axis="y", labelcolor="tab:red")
    # Combine both legends into one.
    lines = left_line + right_line
    labels = [line.get_label() for line in lines]
    axis.legend(lines, labels, loc="best")
    # Label the panel.
    axis.set_title(title)


def plot_control_derivatives(derivatives: dict[str, dict[str, float]]) -> None:
    """Plot the full derivative breakdown for every control surface."""
    # Create one panel per control surface.
    figure, axes = plt.subplots(2, 2, figsize=(12, 9))
    # Plot every coefficient derivative for each control surface.
    for axis, (name, values) in zip(axes.flat, derivatives.items()):
        coefficients = list(COEFFICIENT_NAMES)
        derivative_values = [values[coefficient] for coefficient in coefficients]
        colours = [COEFFICIENT_COLOURS[coefficient] for coefficient in coefficients]
        bars = axis.bar(coefficients, derivative_values, color=colours)
        axis.axhline(0, color="black", linewidth=0.8)
        axis.set_ylabel("dC/ddelta [1/rad]")
        axis.set_title(f"{name} derivatives")
        # Annotate each bar with its derivative value.
        for bar, value in zip(bars, derivative_values):
            axis.annotate(
                f"{value:.3f}",
                (bar.get_x() + bar.get_width() / 2, value),
                ha="center",
                va="bottom" if value >= 0 else "top",
                fontsize=8,
                rotation=90,
            )
    # Add a shared title and tidy the panel spacing.
    figure.suptitle("Control surface derivative breakdown")
    figure.tight_layout(rect=(0, 0, 1, 0.96))
    # Save a high-resolution copy of the control-derivative plots.
    figure.savefig(OUTPUT_CONTROL_DERIVATIVE_PATH, dpi=200, bbox_inches="tight")
    # Report the output location.
    print(f"Saved control-derivative plots to: {OUTPUT_CONTROL_DERIVATIVE_PATH}")


def plot_trajectory(
    name: str,
    times: np.ndarray,
    states: np.ndarray,
    control_history: dict[str, np.ndarray],
    control_name: str,
    output_path: Path,
) -> None:
    """Plot one six-degree-of-freedom trajectory and save the figure."""
    # Extract the position and velocity histories.
    x_e = states[0]
    z_e = states[2]
    u_b = states[3]
    v_b = states[4]
    w_b = states[5]
    # Convert the attitude and rate histories to degrees.
    phi = np.degrees(states[6])
    theta = np.degrees(states[7])
    psi = np.degrees(states[8])
    p = np.degrees(states[9])
    q = np.degrees(states[10])
    r = np.degrees(states[11])
    # Compute the airspeed, altitude and aerodynamic angles.
    speed = np.sqrt(u_b**2 + v_b**2 + w_b**2)
    altitude = -z_e
    alpha = np.degrees(np.arctan2(w_b, u_b))
    beta = np.degrees(np.arctan2(v_b, np.hypot(u_b, w_b)))
    # Create a three-by-two panel of trajectory plots.
    figure, axes = plt.subplots(3, 2, figsize=(13, 11))
    # Plot the active control input.
    axes[0, 0].plot(times, control_history[control_name], color="tab:red")
    axes[0, 0].set_xlabel("Time [s]")
    axes[0, 0].set_ylabel(f"{control_name} deflection [deg]")
    axes[0, 0].set_title(f"{control_name} doublet")
    axes[0, 0].grid(True, alpha=0.3)
    # Plot the Euler angles.
    for values, label in ((phi, "phi"), (theta, "theta"), (psi, "psi")):
        axes[0, 1].plot(times, values, label=label)
    axes[0, 1].set_xlabel("Time [s]")
    axes[0, 1].set_ylabel("Attitude [deg]")
    axes[0, 1].set_title("Euler angles")
    axes[0, 1].legend()
    axes[0, 1].grid(True, alpha=0.3)
    # Plot the body angular rates.
    for values, label in ((p, "p"), (q, "q"), (r, "r")):
        axes[1, 0].plot(times, values, label=label)
    axes[1, 0].set_xlabel("Time [s]")
    axes[1, 0].set_ylabel("Body rate [deg/s]")
    axes[1, 0].set_title("Angular rates")
    axes[1, 0].legend()
    axes[1, 0].grid(True, alpha=0.3)
    # Plot the airspeed on the left axis.
    axes[1, 1].plot(times, speed, color="tab:blue")
    axes[1, 1].set_xlabel("Time [s]")
    axes[1, 1].set_ylabel("Airspeed [m/s]", color="tab:blue")
    axes[1, 1].tick_params(axis="y", labelcolor="tab:blue")
    axes[1, 1].set_title("Airspeed and altitude")
    axes[1, 1].grid(True, alpha=0.3)
    # Plot the altitude on the right axis.
    altitude_axis = axes[1, 1].twinx()
    altitude_axis.plot(times, altitude, color="tab:green")
    altitude_axis.set_ylabel("Altitude [m]", color="tab:green")
    altitude_axis.tick_params(axis="y", labelcolor="tab:green")
    # Plot the aerodynamic angles.
    axes[2, 0].plot(times, alpha, label="alpha")
    axes[2, 0].plot(times, beta, label="beta")
    axes[2, 0].set_xlabel("Time [s]")
    axes[2, 0].set_ylabel("Angle [deg]")
    axes[2, 0].set_title("Aerodynamic angles")
    axes[2, 0].legend()
    axes[2, 0].grid(True, alpha=0.3)
    # Plot the flown path in the vertical plane.
    axes[2, 1].plot(x_e, altitude, color="tab:purple")
    axes[2, 1].set_xlabel("Earth x [m]")
    axes[2, 1].set_ylabel("Altitude [m]")
    axes[2, 1].set_title("Flight path")
    axes[2, 1].grid(True, alpha=0.3)
    # Add a shared title and tidy the panel spacing.
    figure.suptitle(f"Six-degree-of-freedom response: {name} doublet")
    figure.tight_layout(rect=(0, 0, 1, 0.97))
    # Save a high-resolution copy of the trajectory figure.
    figure.savefig(output_path, dpi=200, bbox_inches="tight")
    # Report the output location.
    print(f"Saved {name} trajectory plot to: {output_path}")


def plot_trajectories_3d(
    trajectories: dict[str, tuple[np.ndarray, np.ndarray]],
) -> None:
    """Plot all example trajectories in a three-dimensional view."""
    # Create a single three-dimensional axis.
    figure = plt.figure(figsize=(11, 8))
    axis = figure.add_subplot(111, projection="3d")
    # Plot each trajectory with its own colour.
    for name, (_, states) in trajectories.items():
        axis.plot(states[0], states[1], -states[2], label=name.capitalize())
    # Label the axes and set a readable box shape.
    axis.set_xlabel("Earth x [m]")
    axis.set_ylabel("Earth y [m]")
    axis.set_zlabel("Altitude [m]")
    axis.set_title("Six-degree-of-freedom trajectories")
    axis.set_box_aspect((2.0, 1.0, 0.4))
    axis.legend()
    # Save a high-resolution copy of the combined plot.
    figure.savefig(OUTPUT_TRAJECTORY_3D_PATH, dpi=200, bbox_inches="tight")
    # Report the output location.
    print(f"Saved combined trajectory plot to: {OUTPUT_TRAJECTORY_3D_PATH}")


def main() -> None:
    """Run the aerodynamic study and trajectory simulations, report results and plot them."""
    # Build the complete geometry.
    aircraft = build_aircraft()
    # Use sea-level conditions for the study.
    atmosphere = asb.Atmosphere(altitude=0.0)
    # Build the cruise operating point.
    cruise_op_point = asb.OperatingPoint(
        atmosphere=atmosphere,
        velocity=CRUISE_SPEED,
        alpha=CRUISE_ALPHA,
        beta=CRUISE_BETA,
    )
    # Compute the forces, moments and stability derivatives at cruise.
    cruise_results = run_point_analysis(aircraft, cruise_op_point)
    # Compute the control-surface derivatives at cruise.
    control_derivatives = compute_control_derivatives(aircraft, atmosphere)
    # Print the complete aerodynamic report.
    print_reference_data(aircraft)
    print_force_and_moment_coefficients(cruise_results)
    print_stability_derivatives(cruise_results)
    print_control_derivatives(control_derivatives, control_surface_areas(aircraft))
    # Run the angle-of-attack and sideslip sweeps used for the plots.
    alpha_values = np.linspace(-4.0, 10.0, 15)
    alpha_results = run_sweep(aircraft, atmosphere, alpha_values, CRUISE_BETA)
    beta_values = np.linspace(-8.0, 8.0, 9)
    beta_results = run_sweep(aircraft, atmosphere, CRUISE_ALPHA, beta_values)
    # Run a deflection sweep for every control surface.
    control_results = {
        name: run_control_sweep(aircraft, atmosphere, name, deflections)
        for name, deflections in CONTROL_SWEEP_RANGES.items()
    }
    # Draw the geometry and every coefficient study.
    plot_aircraft(aircraft)
    plot_aero_study(alpha_values, alpha_results, beta_values, beta_results)
    plot_control_surface_response(CONTROL_SWEEP_RANGES, control_results)
    plot_control_derivatives(control_derivatives)
    # Build the mass properties used by the trajectory simulations.
    mass_props = build_mass_properties()
    # Use a constant atmosphere at the initial trajectory altitude.
    trajectory_atmosphere = asb.Atmosphere(altitude=TRAJECTORY_ALTITUDE)
    # Trim the aircraft for level flight.
    trim_alpha, trim_elevator = trim_aircraft(
        aircraft,
        trajectory_atmosphere,
        mass_props,
    )
    # Compute the constant thrust that balances drag at trim.
    thrust = trim_thrust(
        aircraft,
        trajectory_atmosphere,
        mass_props,
        trim_alpha,
        trim_elevator,
    )
    # Report the trajectory setup.
    print_trajectory_setup(mass_props, trim_alpha, trim_elevator, thrust)
    # Build the common initial state for every trajectory.
    initial_state = build_initial_state(trim_alpha)
    # Store each trajectory for the combined plot.
    trajectories = {}
    for name, case in TRAJECTORY_CASES.items():
        # Build the control doublet for this case.
        schedule = build_control_schedule(
            trim_elevator,
            case["control"],
            case["amplitude"],
        )
        # Integrate the six-degree-of-freedom trajectory.
        times, states = simulate_trajectory(
            aircraft,
            mass_props,
            trajectory_atmosphere,
            initial_state,
            schedule,
            thrust,
        )
        # Record the control history at every sample time.
        control_history = {
            control: np.array([schedule(time_value)[control] for time_value in times])
            for control in RECORDED_CONTROLS
        }
        # Store the trajectory for the combined plot.
        trajectories[name] = (times, states)
        # Plot the trajectory response.
        plot_trajectory(
            name,
            times,
            states,
            control_history,
            case["control"],
            OUTPUT_TRAJECTORY_PATHS[name],
        )
        # Report the peak response.
        print_trajectory_summary(name, states)
    # Plot all trajectories together in three dimensions.
    plot_trajectories_3d(trajectories)
    # Display all figures when a GUI backend is available.
    plt.show()


if __name__ == "__main__":
    # Only run the demo when executed as a script.
    main()
