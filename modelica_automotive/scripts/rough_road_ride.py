from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "RoughRoadRide",
        (
            PlotPanel(
                "Four-track road input",
                "Time (s)",
                "Height (m)",
                tuple((f"roadHeight[{i}]", f"Corner {i}") for i in range(1, 5)),
            ),
            PlotPanel(
                "Sprung-body response",
                "Time (s)",
                "Value",
                (("heave", "Heave (m)"), ("roll", "Roll (rad)"), ("pitch", "Pitch (rad)")),
            ),
            PlotPanel(
                "Suspension forces",
                "Time (s)",
                "Force (N)",
                tuple((f"suspensionForce[{i}]", f"Corner {i}") for i in range(1, 5)),
            ),
        ),
        stop_time=8,
        step_size=0.005,
    )


if __name__ == "__main__":
    main()
