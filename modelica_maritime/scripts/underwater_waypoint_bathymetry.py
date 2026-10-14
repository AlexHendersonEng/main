from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "UnderwaterWaypointBathymetry",
        (
            PlotPanel(
                "Underwater route",
                "East (m)",
                "North (m)",
                (("positionNED[1]", "Vehicle path"),),
                x_variable="positionNED[2]",
            ),
            PlotPanel(
                "Waypoint range and seafloor altitude",
                "Time (s)",
                "Value",
                (("distance", "Waypoint range (m)"), ("altitude", "Altitude (m)")),
            ),
            PlotPanel(
                "Actual and commanded depth",
                "Time (s)",
                "Depth (m)",
                (("positionNED[3]", "Depth"), ("commandedDepth", "Commanded")),
            ),
        ),
        stop_time=100,
        step_size=0.1,
    )


if __name__ == "__main__":
    main()
