from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "PropulsionFailureResponse",
        (
            PlotPanel(
                "Surface-vessel path",
                "East (m)",
                "North (m)",
                (("poseNED[2]", "Vessel path"),),
                x_variable="poseNED[1]",
            ),
            PlotPanel(
                "Cross-track and heading errors",
                "Time (s)",
                "Error",
                (("crossTrackError", "Cross-track (m)"), ("headingError", "Heading (rad)")),
            ),
            PlotPanel(
                "Body velocities",
                "Time (s)",
                "Velocity",
                tuple((f"velocityBody[{i}]", f"DOF {i}") for i in range(1, 4)),
            ),
        ),
        stop_time=80,
        step_size=0.1,
    )


if __name__ == "__main__":
    main()
