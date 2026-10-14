from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "DoubleTrackCornering",
        (
            PlotPanel(
                "Braking-turn path",
                "World X (m)",
                "World Y (m)",
                (("position[2]", "Vehicle path"),),
                x_variable="position[1]",
            ),
            PlotPanel(
                "Longitudinal and lateral velocity",
                "Time (s)",
                "Velocity (m/s)",
                (
                    ("longitudinalVelocity", "Longitudinal"),
                    ("lateralVelocity", "Lateral"),
                ),
            ),
            PlotPanel(
                "Four-corner normal loads",
                "Time (s)",
                "Load (N)",
                tuple((f"normalLoad[{i}]", f"Corner {i}") for i in range(1, 5)),
            ),
            PlotPanel(
                "Yaw response",
                "Time (s)",
                "Value",
                (("yaw", "Yaw (rad)"), ("yawRate", "Yaw rate (rad/s)")),
            ),
            PlotPanel(
                "Torque-vectoring yaw moment",
                "Time (s)",
                "Moment (N.m)",
                (("yawMomentCommand", "Yaw moment"),),
            ),
        ),
        stop_time=8,
        step_size=0.01,
    )


if __name__ == "__main__":
    main()
