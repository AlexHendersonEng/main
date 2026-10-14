from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "SixDegreeOfFreedomAircraft",
        (
            PlotPanel(
                "Ground track",
                "East distance (m)",
                "North distance (m)",
                (("positionNED[1]", "Aircraft path"),),
                x_variable="positionNED[2]",
            ),
            PlotPanel(
                "Attitude",
                "Time (s)",
                "Angle (rad)",
                (("euler321[1]", "Roll"), ("euler321[2]", "Pitch"), ("euler321[3]", "Yaw")),
            ),
            PlotPanel(
                "Body angular rates",
                "Time (s)",
                "Rate (rad/s)",
                (
                    ("angularVelocityBody[1]", "Roll rate"),
                    ("angularVelocityBody[2]", "Pitch rate"),
                    ("angularVelocityBody[3]", "Yaw rate"),
                ),
            ),
        ),
        stop_time=8,
        step_size=0.02,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
