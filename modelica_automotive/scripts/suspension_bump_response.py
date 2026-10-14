from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "SuspensionBumpResponse",
        (
            PlotPanel(
                "Sprung-body motion",
                "Time (s)",
                "Value",
                (("heave", "Heave (m)"), ("roll", "Roll (rad)"), ("pitch", "Pitch (rad)")),
            ),
            PlotPanel(
                "Corner suspension force",
                "Time (s)",
                "Force (N)",
                tuple((f"suspensionForce[{i}]", f"Corner {i}") for i in range(1, 5)),
            ),
            PlotPanel(
                "Left-front road bump",
                "Time (s)",
                "Height (m)",
                (("roadHeight[1]", "Road height"),),
            ),
        ),
        stop_time=5,
        step_size=0.005,
    )


if __name__ == "__main__":
    main()
