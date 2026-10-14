from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "SplitFrictionABS",
        (
            PlotPanel("Vehicle speed", "Time (s)", "Speed (m/s)", (("speed", "Speed"),)),
            PlotPanel(
                "Wheel slip",
                "Time (s)",
                "Slip ratio",
                (("slipRatio[1]", "Left"), ("slipRatio[2]", "Right")),
            ),
            PlotPanel(
                "ABS commands",
                "Time (s)",
                "Brake command",
                (("brakeCommand[1]", "Left"), ("brakeCommand[2]", "Right")),
            ),
        ),
        stop_time=4,
        step_size=0.005,
    )


if __name__ == "__main__":
    main()
