from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "TireSweep",
        (
            PlotPanel(
                "Longitudinal tire force vs slip ratio",
                "Slip ratio",
                "Force (N)",
                (
                    ("linearForce[1]", "Linear"),
                    ("fialaForce[1]", "Fiala"),
                    ("magicFormulaForce[1]", "Magic Formula"),
                ),
                x_variable="slipRatio",
            ),
            PlotPanel(
                "Lateral tire force vs slip angle",
                "Slip angle (rad)",
                "Force (N)",
                (
                    ("linearForce[2]", "Linear"),
                    ("fialaForce[2]", "Fiala"),
                    ("magicFormulaForce[2]", "Magic Formula"),
                ),
                x_variable="slipAngle",
            ),
        ),
        stop_time=4,
        step_size=0.01,
    )


if __name__ == "__main__":
    main()
