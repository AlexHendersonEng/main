from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "SphericalOrbit",
        (
            PlotPanel(
                "ECEF orbital ground-plane projection",
                "ECEF x (m)",
                "ECEF y (m)",
                (("positionECEF[2]", "Orbit"),),
                x_variable="positionECEF[1]",
            ),
            PlotPanel(
                "Geodetic altitude",
                "Time (s)",
                "Altitude (m)",
                (("altitude", "Altitude"),),
            ),
            PlotPanel(
                "Specific mechanical energy",
                "Time (s)",
                "Energy (m2/s2)",
                (("specificMechanicalEnergy", "Energy"),),
            ),
        ),
        stop_time=6000,
        step_size=5,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
