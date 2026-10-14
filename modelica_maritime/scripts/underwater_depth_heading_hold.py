from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "UnderwaterDepthHeadingHold",
        (
            PlotPanel(
                "Depth tracking",
                "Time (s)",
                "Depth (m)",
                (("positionNED[3]", "Depth"),),
            ),
            PlotPanel(
                "Heading error",
                "Time (s)",
                "Error (rad)",
                (("headingError", "Heading error"),),
            ),
            PlotPanel(
                "Body velocities",
                "Time (s)",
                "Velocity",
                tuple((f"velocityBody[{i}]", f"DOF {i}") for i in range(1, 7)),
            ),
        ),
        stop_time=80,
        step_size=0.1,
    )


if __name__ == "__main__":
    main()
