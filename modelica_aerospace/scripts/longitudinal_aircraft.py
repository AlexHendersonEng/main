from _example_runner import PlotPanel, run_example


def main() -> None:
    run_example(
        "LongitudinalAircraft",
        (
            PlotPanel(
                "Altitude",
                "Time (s)",
                "Altitude (m)",
                (("altitudeOutput", "Altitude"),),
            ),
            PlotPanel(
                "Airspeed",
                "Time (s)",
                "Speed (m/s)",
                (("speed", "Airspeed"),),
            ),
            PlotPanel(
                "Engine response",
                "Time (s)",
                "Thrust (N)",
                (("thrust", "Thrust"),),
            ),
            PlotPanel(
                "Flight-path angle",
                "Time (s)",
                "Angle (rad)",
                (("flightPathAngle", "Flight-path angle"),),
            ),
            PlotPanel(
                "Mach number",
                "Time (s)",
                "Mach",
                (("mach", "Mach"),),
            ),
        ),
        stop_time=15,
        step_size=0.05,
        tolerance=1e-8,
    )


if __name__ == "__main__":
    main()
