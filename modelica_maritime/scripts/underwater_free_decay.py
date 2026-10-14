from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "UnderwaterFreeDecay",
        (
            PlotPanel(
                "Attitude quaternion",
                "Time (s)",
                "Component",
                tuple((f"quaternion[{i}]", f"q{i}") for i in range(1, 5)),
            ),
            PlotPanel(
                "Body translational velocity",
                "Time (s)",
                "Velocity (m/s)",
                tuple((f"velocityBody[{i}]", f"Axis {i}") for i in range(1, 4)),
            ),
            PlotPanel(
                "Hydrodynamic dissipation",
                "Time (s)",
                "Power (W)",
                (("dissipationPower", "Dissipation"),),
            ),
        ),
        stop_time=40,
        step_size=0.05,
    )


if __name__ == "__main__":
    main()
