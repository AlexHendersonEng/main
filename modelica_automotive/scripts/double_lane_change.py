from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "DoubleLaneChange",
        (
            PlotPanel(
                "Lane-change path",
                "Longitudinal position (m)",
                "Lateral position (m)",
                (("position[2]", "Vehicle path"), ("targetLateralPosition", "Path target")),
                x_variable="position[1]",
            ),
            PlotPanel(
                "Path tracking error",
                "Time (s)",
                "Error (m)",
                (("lateralError", "Lateral error"),),
            ),
            PlotPanel(
                "Steering command",
                "Time (s)",
                "Angle (rad)",
                (("steeringCommand", "Steering"),),
            ),
        ),
        stop_time=8,
        step_size=0.01,
    )


if __name__ == "__main__":
    main()
