"""Launch a :class:`~polaris.cosim.config.CoSimulation` as running HELICS federates.

This module owns the HELICS broker lifecycle and concurrency required to execute a
federation: federates that exchange values must run concurrently because HELICS time
grants block until every federate has requested at least that time (see
:mod:`polaris.cosim.federate`). :func:`run_cosimulation` hides that behind a single call
that returns one :class:`~polaris.result.Result` per federate.
"""

from __future__ import annotations

import importlib
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor
from typing import Any
from uuid import uuid4

from polaris.cosim.config import CoSimulation, FederateSpec
from polaris.cosim.federate import FmuFederate
from polaris.result import Result


def run_cosimulation(config: CoSimulation, *, workers: int = 1) -> dict[str, Result]:
    """Run every federate in ``config`` concurrently and return their results.

    Starts a HELICS broker sized for the federation, builds an
    :class:`~polaris.cosim.federate.FmuFederate` per :class:`~polaris.cosim.config.FederateSpec`
    with inputs/outputs derived from ``config.connections``, runs them all in a thread
    pool, and tears down the broker and federates afterwards even if a federate fails.
    When ``workers`` is greater than one, federates are partitioned across worker
    processes; each process still runs its assigned federates concurrently in threads.

    Args:
        config: The federation to run.
        workers: Maximum number of worker processes. ``1`` keeps execution in this process.

    Returns:
        Federate name -> its simulation :class:`~polaris.result.Result`.

    Raises:
        ImportError: HELICS bindings are not installed; install ``polaris[helics]``.
        ValueError: ``workers`` is invalid or an in-process HELICS core was requested
            for multi-process execution.
    """
    if workers < 1:
        raise ValueError("workers must be at least 1")
    if workers > 1 and (
        config.broker_core_type == "inproc"
        or any(spec.core_type == "inproc" for spec in config.federates)
    ):
        raise ValueError("HELICS core type 'inproc' cannot connect separate worker processes")

    try:
        helics = importlib.import_module("helics")
    except ImportError as exc:
        raise ImportError(
            "HELICS support is optional; install it with `pip install polaris[helics]`."
        ) from exc

    broker_name = config.broker_name or f"polaris-cosim-{uuid4().hex[:8]}"
    broker_init = config.broker_init or f"--federates={len(config.federates)}"
    broker = helics.helicsCreateBroker(config.broker_core_type, broker_name, broker_init)
    if broker is None:
        raise RuntimeError("Could not start the HELICS broker")

    if workers > 1:
        try:
            broker_address = helics.helicsBrokerGetAddress(broker)
            return _run_in_processes(config, broker_address, workers)
        finally:
            _shutdown_broker(helics, broker)

    federates: dict[str, FmuFederate] = {}
    try:
        for spec in config.federates:
            federates[spec.name] = FmuFederate(
                spec.fmu,
                spec.name,
                inputs=config.inputs_for(spec.name),
                outputs=config.outputs_for(spec.name),
                step_size=spec.step_size,
                substeps=spec.substeps,
                start_time=spec.start_time,
                parameters=spec.parameters,
                broker=broker_name,
                core_type=spec.core_type,
                core_init=spec.core_init,
                compile_sources=spec.compile_sources,
            )
        # Federates exchange values through HELICS time grants, so they must all be
        # advancing concurrently; running them one after another would deadlock.
        with ThreadPoolExecutor(max_workers=len(federates)) as pool:
            futures = {
                name: pool.submit(federate.run, config.stop_time)
                for name, federate in federates.items()
            }
            return {name: future.result() for name, future in futures.items()}
    finally:
        for federate in federates.values():
            federate.close()
        _shutdown_broker(helics, broker)


def _run_in_processes(config: CoSimulation, broker_address: str, workers: int) -> dict[str, Result]:
    """Partition specs among processes; each process runs its partition on threads."""
    partitions: list[list[FederateSpec]] = [[] for _ in range(min(workers, len(config.federates)))]
    for index, spec in enumerate(config.federates):
        partitions[index % len(partitions)].append(spec)

    with ProcessPoolExecutor(max_workers=len(partitions)) as pool:
        futures = [
            pool.submit(
                _run_federate_partition,
                config,
                partition,
                broker_address,
            )
            for partition in partitions
        ]
        results: dict[str, Result] = {}
        for future in futures:
            results.update(future.result())
    return results


def _run_federate_partition(
    config: CoSimulation,
    specs: list[FederateSpec],
    broker_address: str,
) -> dict[str, Result]:
    """Create and concurrently run one worker process's assigned federates."""
    federates: dict[str, FmuFederate] = {}
    try:
        for spec in specs:
            federates[spec.name] = FmuFederate(
                spec.fmu,
                spec.name,
                inputs=config.inputs_for(spec.name),
                outputs=config.outputs_for(spec.name),
                step_size=spec.step_size,
                substeps=spec.substeps,
                start_time=spec.start_time,
                parameters=spec.parameters,
                broker=broker_address,
                core_type=spec.core_type,
                core_init=spec.core_init,
                compile_sources=spec.compile_sources,
            )
        with ThreadPoolExecutor(max_workers=len(federates)) as pool:
            futures = {
                name: pool.submit(federate.run, config.stop_time)
                for name, federate in federates.items()
            }
            return {name: future.result() for name, future in futures.items()}
    finally:
        for federate in federates.values():
            federate.close()


def _shutdown_broker(helics: Any, broker: Any) -> None:
    """Wait for federates to disconnect and release the broker handle."""
    helics.helicsBrokerWaitForDisconnect(broker, 5000)
    helics.helicsBrokerFree(broker)
