from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "AtmosphereGeodesy",
        (
            PlotPanel(
                "Atmospheric temperature",
                "Time (s)",
                "Temperature (K)",
                (("temperature", "Temperature"),),
            ),
            PlotPanel(
                "Speed of sound",
                "Time (s)",
                "Speed (m/s)",
                (("speedOfSound", "Speed of sound"),),
            ),
            PlotPanel("Static pressure", "Time (s)", "Pressure (Pa)", (("pressure", "Pressure"),)),
            PlotPanel("Air density", "Time (s)", "Density (kg/m3)", (("density", "Density"),)),
        ),
        stop_time=0.1,
        step_size=0.1,
        tolerance=1e-9,
    )


if __name__ == "__main__":
    main()
