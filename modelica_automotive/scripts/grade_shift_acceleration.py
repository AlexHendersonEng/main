from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "GradeShiftAcceleration",
        (
            PlotPanel("Uphill speed", "Time (s)", "Speed (m/s)", (("speed", "Speed"),)),
            PlotPanel(
                "Drive force and acceleration",
                "Time (s)",
                "Value",
                (("driveForce", "Drive force (N)"), ("acceleration", "Acceleration (m/s2)")),
            ),
            PlotPanel(
                "Machine torque and battery state",
                "Time (s)",
                "Value",
                (("machineTorque", "Torque (N.m)"), ("stateOfCharge", "State of charge")),
            ),
        ),
        stop_time=12,
        step_size=0.01,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
