from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "LongitudinalDrive",
        (
            PlotPanel("Vehicle speed", "Time (s)", "Speed (m/s)", (("speed", "Speed"),)),
            PlotPanel(
                "Acceleration and tire force",
                "Time (s)",
                "Value",
                (("acceleration", "Acceleration (m/s2)"), ("tireForce", "Tire force (N)")),
            ),
            PlotPanel("Distance", "Time (s)", "Position (m)", (("position", "Position"),)),
        ),
        stop_time=20,
        step_size=0.05,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
