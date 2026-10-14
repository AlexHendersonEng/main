from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "SurfaceWaveResponse",
        (
            PlotPanel(
                "Wave elevation and heave",
                "Time (s)",
                "Elevation (m)",
                (("waveElevation", "Wave"), ("positionNED[3]", "Vehicle depth (m)")),
            ),
            PlotPanel(
                "NED position",
                "Time (s)",
                "Position (m)",
                tuple((f"positionNED[{i}]", f"Axis {i}") for i in range(1, 4)),
            ),
            PlotPanel(
                "Wave generalized loads",
                "Time (s)",
                "Load",
                tuple((f"waveLoadBody[{i}]", f"DOF {i}") for i in range(1, 7)),
            ),
        ),
        stop_time=60,
        step_size=0.05,
    )


if __name__ == "__main__":
    main()
