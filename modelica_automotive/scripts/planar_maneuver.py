from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "PlanarManeuver",
        (
            PlotPanel(
                "Steering command and response",
                "Time (s)",
                "Angle (rad)",
                (("steeringCommand", "Command"), ("steeringAngle", "Actual")),
            ),
            PlotPanel(
                "Yaw response",
                "Time (s)",
                "Value",
                (("yaw", "Yaw (rad)"), ("yawRate", "Yaw rate (rad/s)")),
            ),
            PlotPanel(
                "Lateral acceleration",
                "Time (s)",
                "Acceleration (m/s2)",
                (("lateralAcceleration", "Lateral acceleration"),),
            ),
        ),
        stop_time=10,
        step_size=0.01,
    )


if __name__ == "__main__":
    main()
