from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "MMGTurningCircle",
        (
            PlotPanel(
                "MMG turning-circle path",
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
                "Hull, propeller, and rudder surge loads",
                "Time (s)",
                "Load (N)",
                (
                    ("hullLoadBody[1]", "Hull"),
                    ("propellerLoadBody[1]", "Propeller"),
                    ("rudderLoadBody[1]", "Rudder"),
                ),
            ),
        ),
        stop_time=240,
        step_size=0.2,
    )


if __name__ == "__main__":
    main()
