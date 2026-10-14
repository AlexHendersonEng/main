from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "RegenerativeBraking",
        (
            PlotPanel("Vehicle speed", "Time (s)", "Speed (m/s)", (("speed", "Speed"),)),
            PlotPanel(
                "Regenerative power",
                "Time (s)",
                "Power (W)",
                (("sourcePower", "Machine source power"),),
            ),
            PlotPanel(
                "Battery state of charge",
                "Time (s)",
                "State of charge",
                (("stateOfCharge", "State of charge"),),
            ),
        ),
        stop_time=8,
        step_size=0.01,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
