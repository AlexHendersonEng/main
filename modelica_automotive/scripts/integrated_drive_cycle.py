from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "IntegratedDriveCycle",
        (
            PlotPanel(
                "Target and actual speed",
                "Time (s)",
                "Speed (m/s)",
                (("targetSpeed", "Target"), ("speed", "Actual")),
            ),
            PlotPanel(
                "Drive and brake commands",
                "Time (s)",
                "Command",
                (
                    ("propulsionCommand", "Propulsion"),
                    ("regenerativeCommand", "Regenerative"),
                    ("frictionBrakeCommand", "Friction brake"),
                ),
            ),
            PlotPanel(
                "Storage energy state and power",
                "Time (s)",
                "Value",
                (("stateOfCharge", "State of charge"), ("sourcePower", "Source power (W)")),
            ),
        ),
        stop_time=24,
        step_size=0.02,
    )


if __name__ == "__main__":
    main()
