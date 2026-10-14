from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "TractionControlledLaunch",
        (
            PlotPanel("Vehicle speed", "Time (s)", "Speed (m/s)", (("speed", "Speed"),)),
            PlotPanel(
                "Driven-wheel slip",
                "Time (s)",
                "Slip ratio",
                (("slipRatio[1]", "Left"), ("slipRatio[2]", "Right")),
            ),
            PlotPanel(
                "Traction-control command and tire force",
                "Time (s)",
                "Value",
                (
                    ("propulsionCommand", "Propulsion command"),
                    ("tireForce[1]", "Left tire force (N)"),
                    ("tireForce[2]", "Right tire force (N)"),
                ),
            ),
        ),
        stop_time=6,
        step_size=0.005,
    )


if __name__ == "__main__":
    main()
