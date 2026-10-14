from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "StabilityControlledTurn",
        (
            PlotPanel(
                "Vehicle path",
                "World X (m)",
                "World Y (m)",
                (("position[2]", "Path"),),
                x_variable="position[1]",
            ),
            PlotPanel(
                "Desired and measured yaw rate",
                "Time (s)",
                "Rate (rad/s)",
                (("desiredYawRate", "Desired"), ("yawRate", "Measured")),
            ),
            PlotPanel(
                "Yaw moment command and disturbance",
                "Time (s)",
                "Moment (N.m)",
                (("yawMomentCommand", "Control"), ("disturbanceMoment", "Disturbance")),
            ),
        ),
        stop_time=8,
        step_size=0.01,
    )


if __name__ == "__main__":
    main()
