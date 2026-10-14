from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "DrydenGustResponse",
        (
            PlotPanel(
                "Body-axis Dryden turbulence",
                "Time (s)",
                "Turbulence velocity (m/s)",
                (
                    ("turbulenceBody[1]", "Longitudinal"),
                    ("turbulenceBody[2]", "Lateral"),
                    ("turbulenceBody[3]", "Vertical"),
                ),
            ),
            PlotPanel(
                "Vertical response",
                "Time (s)",
                "Vertical velocity (m/s)",
                (("verticalVelocity", "Aircraft response"),),
            ),
        ),
        stop_time=40,
        step_size=0.05,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
