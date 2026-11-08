"""Build a simple general-aviation aircraft and draw it with AeroSandbox."""

from pathlib import Path
import aerosandbox as asb
import matplotlib.pyplot as plt

# Save the render beside this script.
OUTPUT_PATH = Path(__file__).resolve().parent / "aircraft_demo.png"
# Use a cambered aerofoil for the main wing.
MAIN_AIRFOIL = asb.Airfoil("naca2412")
# Use a symmetric aerofoil for both tail surfaces.
TAIL_AIRFOIL = asb.Airfoil("naca0012")


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
    """Build a tapered, high-mounted main wing with dihedral and washout."""
    # Mirror the half-wing about the symmetry plane.
    return asb.Wing(
        name="Main wing",
        symmetric=True,
        xsecs=[
            # Root section, mounted above the cabin.
            asb.WingXSec(
                xyz_le=[1.90, 0.00, 0.95],
                chord=1.60,
                twist=2.0,
                airfoil=MAIN_AIRFOIL,
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
    """Build the horizontal stabiliser."""
    # Mirror the half-tail about the symmetry plane.
    return asb.Wing(
        name="Horizontal stabiliser",
        symmetric=True,
        xsecs=[
            # Root section, blended into the tail cone.
            asb.WingXSec(
                xyz_le=[5.95, 0.00, 0.35],
                chord=0.85,
                airfoil=TAIL_AIRFOIL,
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
    """Build the vertical stabiliser."""
    # Define root and tip sections; the span runs upwards.
    return asb.Wing(
        name="Vertical stabiliser",
        xsecs=[
            # Root section, blended into the tail cone.
            asb.WingXSec(
                xyz_le=[5.90, 0, 0.30],
                chord=1.05,
                airfoil=TAIL_AIRFOIL,
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
    """Assemble the fuselage, lifting surfaces and propeller into one aircraft."""
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
                xyz_c=[0.0, 0.0, 0.0],
                xyz_normal=[-1.0, 0.0, 0.0],
                radius=0.78,
                length=0.0,
            ),
        ],
    )


def plot_aircraft(aircraft: asb.Airplane) -> None:
    """Draw the aircraft in an isometric view and save the figure."""
    # Draw the complete aircraft with AeroSandbox, deferring display.
    aircraft.draw(
        backend="matplotlib",
        use_preset_view_angle="right_isometric",
        show=False,
    )
    # Label the view with the aircraft name.
    plt.gca().set_title(aircraft.name)
    # Save a high-resolution render.
    plt.savefig(OUTPUT_PATH, dpi=200, bbox_inches="tight")
    # Report the output location.
    print(f"Saved aircraft render to: {OUTPUT_PATH}")
    # Display the interactive view when a GUI backend is available.
    plt.show()


def main() -> None:
    """Build the aircraft, report its size and plot it."""
    # Build the complete geometry.
    aircraft = build_aircraft()
    # Retrieve the main wing and fuselage for the size summary.
    main_wing = aircraft.wings[0]
    fuselage = aircraft.fuselages[0]
    # Report the full wing span.
    print(f"Main wing span: {main_wing.span():.2f} m")
    # Report the reference wing area.
    print(f"Main wing area: {main_wing.area():.2f} m²")
    # Report the wing aspect ratio.
    print(f"Main wing aspect ratio: {main_wing.aspect_ratio():.2f}")
    # Report the overall fuselage length.
    print(f"Fuselage length: {fuselage.length():.2f} m")
    # Draw and save the aircraft.
    plot_aircraft(aircraft)


if __name__ == "__main__":
    # Only run the demo when executed as a script.
    main()
