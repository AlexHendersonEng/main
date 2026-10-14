from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "SurfaceCurrentCompensation",
        (
            PlotPanel(
                "Route tracking in cross-current",
                "East (m)",
                "North (m)",
                (("poseNED[2]", "Vessel path"),),
                x_variable="poseNED[1]",
            ),
            PlotPanel(
                "Ground-track command and compensated heading",
                "Time (s)",
                "Heading (rad)",
                (
                    ("commandedGroundTrack", "Ground track"),
                    ("commandedHeading", "Through-water heading"),
                ),
            ),
            PlotPanel(
                "Cross-track and heading error",
                "Time (s)",
                "Error",
                (("crossTrackError", "Cross-track (m)"), ("headingError", "Heading (rad)")),
            ),
        ),
        stop_time=80,
        step_size=0.1,
    )


if __name__ == "__main__":
    main()
