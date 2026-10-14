from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "SurfaceManeuvering",
        (
            PlotPanel(
                "Surface-vessel ground track",
                "East (m)",
                "North (m)",
                (("poseNED[1]", "Vessel path"),),
                x_variable="poseNED[2]",
            ),
            PlotPanel(
                "Body velocities",
                "Time (s)",
                "Velocity (m/s)",
                tuple((f"velocityBody[{i}]", f"Axis {i}") for i in range(1, 4)),
            ),
            PlotPanel(
                "Relative surge and dissipation",
                "Time (s)",
                "Value",
                (
                    ("relativeVelocityBody[1]", "Relative surge (m/s)"),
                    ("dissipationPower", "Dissipation (W)"),
                ),
            ),
        ),
        stop_time=60,
        step_size=0.1,
    )


if __name__ == "__main__":
    main()
