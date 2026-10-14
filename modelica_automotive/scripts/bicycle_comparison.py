from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "BicycleComparison",
        (
            PlotPanel(
                "Yaw response",
                "Time (s)",
                "Yaw (rad)",
                (("kinematicYaw", "Kinematic"), ("dynamicYaw", "Dynamic")),
            ),
            PlotPanel(
                "Lateral path response",
                "Time (s)",
                "Position Y (m)",
                (("kinematicPosition[2]", "Kinematic"), ("dynamicPosition[2]", "Dynamic")),
            ),
        ),
        stop_time=8,
        step_size=0.02,
    )


if __name__ == "__main__":
    main()
