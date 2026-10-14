from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "WaypointFollowing",
        (
            PlotPanel(
                "Horizontal ground track",
                "East distance (m)",
                "North distance (m)",
                (("positionNED[1]", "Aircraft path"),),
                x_variable="positionNED[2]",
            ),
            PlotPanel(
                "Waypoint range",
                "Time (s)",
                "Distance (m)",
                (("distanceToWaypoint", "Range"),),
            ),
            PlotPanel(
                "Heading guidance",
                "Time (s)",
                "Heading (rad)",
                (("groundTrack", "Ground track"), ("commandedHeading", "Commanded heading")),
            ),
        ),
        stop_time=30,
        step_size=0.05,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
