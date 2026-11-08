"""Build a simple general-aviation aircraft and run an aerodynamic study with AeroSandbox."""

from pathlib import Path
import aerosandbox as asb
import matplotlib.pyplot as plt
import numpy as np

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


def main() -> None:
    """Build the aircraft, run the aerodynamic study, report results and plot them."""
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
    # Display all figures when a GUI backend is available.
    plt.show()


if __name__ == "__main__":
    # Only run the demo when executed as a script.
    main()
