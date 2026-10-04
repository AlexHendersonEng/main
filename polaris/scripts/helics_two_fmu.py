"""Run two Decay FMUs as connected HELICS value federates.

Install the optional bindings first:

    uv sync --extra helics
    uv run --extra helics python scripts/helics_two_fmu.py

The producer publishes its ``y`` output, which the consumer connects to its ``u`` input.
Both federates run in threads because HELICS time grants synchronize them.
"""

from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from uuid import uuid4

import helics
from matplotlib.figure import Figure

from polaris import Model
from polaris.backends import available_backends
from polaris.cosim import FmuFederate
from polaris.plotting import plot
from polaris.result import Result

HERE = Path(__file__).parent


def plot_coupling(producer: Result, consumer: Result, save_to: Path) -> Figure:
    """Plot the producer's published output next to the consumer's received input."""
    fig = Figure(figsize=(8, 5), layout="constrained")
    ax = fig.subplots()
    ax.plot(producer.time, producer["y"], label="producer.y (published)")
    ax.plot(consumer.time, consumer["u"], "--", label="consumer.u (received)")
    ax.set_xlabel("time")
    ax.set_ylabel("value")
    ax.grid(True)
    ax.legend()
    fig.suptitle("Coupling: producer.y -> consumer.u")
    fig.savefig(save_to)
    return fig


def main() -> None:
    """Export two FMUs, connect one output to the other's input, and run the federation."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--backend",
        choices=available_backends(),
        default=None,
        help=(
            "Backend used to export both FMUs "
            "(default: Rumoca if installed, otherwise first available)"
        ),
    )
    parser.add_argument("--stop", type=float, default=2.0, help="Federation stop time")
    parser.add_argument("--step", type=float, default=0.05, help="HELICS/FMU communication step")
    args = parser.parse_args()

    backend = args.backend or ("rumoca" if "rumoca" in available_backends() else None)
    model = Model("Decay", [HERE.parent / "tests" / "models" / "Decay.mo"])
    broker_name = f"polaris-example-{uuid4().hex[:8]}"
    broker = helics.helicsCreateBroker("zmq", broker_name, "--federates=2")
    if broker is None:
        raise RuntimeError("Could not start the HELICS broker")

    producer: FmuFederate | None = None
    consumer: FmuFederate | None = None
    try:
        # Each FMU is exported independently; source-only Rumoca FMUs are compiled locally.
        producer_fmu = model.export_fmu(HERE / "output" / "helics_producer.fmu", backend=backend)
        consumer_fmu = model.export_fmu(HERE / "output" / "helics_consumer.fmu", backend=backend)
        producer = FmuFederate(
            producer_fmu,
            "producer",
            outputs={"y": "producer.y"},
            step_size=args.step,
            broker=broker_name,
        )
        consumer = FmuFederate(
            consumer_fmu,
            "consumer",
            inputs={"u": "producer.y"},
            outputs={"y": "consumer.y"},
            step_size=args.step,
            broker=broker_name,
        )
        # A federate blocks at HELICS time requests until its peers progress as well.
        with ThreadPoolExecutor(max_workers=2) as pool:
            producer_future = pool.submit(producer.run, args.stop)
            consumer_future = pool.submit(consumer.run, args.stop)
            producer_result = producer_future.result()
            consumer_result = consumer_future.result()
        output = HERE / "output"
        output.mkdir(parents=True, exist_ok=True)
        producer_result.save(output / "helics_producer.csv")
        consumer_result.save(output / "helics_consumer.csv")
        print(f"Producer final y: {producer_result['y'][-1]:.6f}")
        print(f"Consumer final x: {consumer_result['x'][-1]:.6f}")

        plot(
            producer_result,
            ["x", "y"],
            separate=True,
            title="Producer (Decay)",
            save_to=output / "helics_producer.png",
        )
        plot(
            consumer_result,
            ["x", "u", "y"],
            separate=True,
            title="Consumer (Decay, driven by producer.y)",
            save_to=output / "helics_consumer.png",
        )
        plot_coupling(producer_result, consumer_result, output / "helics_coupling.png")
        print(f"Results and plots written to {output}")
    finally:
        if producer is not None:
            producer.close()
        if consumer is not None:
            consumer.close()
        helics.helicsBrokerWaitForDisconnect(broker, 5000)
        helics.helicsBrokerFree(broker)


if __name__ == "__main__":
    main()
