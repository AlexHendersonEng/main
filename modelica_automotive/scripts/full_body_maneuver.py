from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "FullBodyManeuver",
        (
            PlotPanel(
                "World path",
                "World X (m)",
                "World Y (m)",
                (("positionWorld[2]", "Path"),),
                x_variable="positionWorld[1]",
            ),
            PlotPanel(
                "Body velocity",
                "Time (s)",
                "Velocity (m/s)",
                tuple((f"velocityBody[{i}]", f"Axis {i}") for i in range(1, 4)),
            ),
            PlotPanel(
                "Body angular velocity",
                "Time (s)",
                "Rate (rad/s)",
                tuple((f"angularVelocityBody[{i}]", f"Axis {i}") for i in range(1, 4)),
            ),
        ),
        stop_time=5,
        step_size=0.01,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
