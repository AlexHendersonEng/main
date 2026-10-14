from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "RoadGradeDrive",
        (
            PlotPanel(
                "Crowned, banked road profile",
                "Longitudinal position (m)",
                "Road height (m)",
                (("roadHeight", "Road height"),),
                x_variable="position",
            ),
            PlotPanel("Vehicle speed", "Time (s)", "Speed (m/s)", (("speed", "Speed"),)),
            PlotPanel(
                "Acceleration and grade load",
                "Time (s)",
                "Value",
                (("acceleration", "Acceleration (m/s2)"), ("gradeForce", "Grade force (N)")),
            ),
        ),
        stop_time=16,
        step_size=0.02,
    )


if __name__ == "__main__":
    main()
