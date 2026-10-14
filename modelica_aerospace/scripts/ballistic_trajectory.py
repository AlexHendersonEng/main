from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "BallisticTrajectory",
        (
            PlotPanel(
                "Trajectory",
                "North distance (m)",
                "Altitude (m)",
                (("altitude", "Ballistic path"),),
                x_variable="positionNED[1]",
            ),
            PlotPanel(
                "Velocity components",
                "Time (s)",
                "Velocity (m/s)",
                (("velocityNED[1]", "North"), ("velocityNED[3]", "Down")),
            ),
            PlotPanel(
                "Specific mechanical energy",
                "Time (s)",
                "Energy (m2/s2)",
                (("specificMechanicalEnergy", "Energy"),),
            ),
        ),
        stop_time=20,
        step_size=0.1,
        tolerance=1e-9,
    )


if __name__ == "__main__":
    main()
