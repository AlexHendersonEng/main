from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "PackageSmoke",
        (PlotPanel("Constant output", "Time (s)", "Output", (("y", "y"),)),),
        stop_time=0.1,
        step_size=0.1,
    )


if __name__ == "__main__":
    main()
