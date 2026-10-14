from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "SplitFrictionBraking",
        (
            PlotPanel("Vehicle speed", "Time (s)", "Speed (m/s)", (("speed", "Speed"),)),
            PlotPanel(
                "Left/right tire forces",
                "Time (s)",
                "Force (N)",
                (("leftLongitudinalForce", "Left"), ("rightLongitudinalForce", "Right")),
            ),
            PlotPanel("Yaw moment", "Time (s)", "Moment (N.m)", (("yawMoment", "Yaw moment"),)),
        ),
        stop_time=3,
        step_size=0.005,
    )


if __name__ == "__main__":
    main()
