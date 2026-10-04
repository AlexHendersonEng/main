"""HELICS FMI integration tests; optional compiler and HELICS dependencies are skipped."""

from __future__ import annotations

import shutil
from concurrent.futures import ThreadPoolExecutor
from importlib.util import find_spec
from pathlib import Path
from uuid import uuid4

import pytest

from polaris import Model
from polaris.cosim import FmuFederate

MODEL_FILE = Path(__file__).parent / "models" / "Decay.mo"
pytestmark = pytest.mark.skipif(find_spec("helics") is None, reason="install polaris[helics]")
needs_backend_and_compiler = pytest.mark.skipif(
    not (shutil.which("omc") or shutil.which("rumoca"))
    or not any(shutil.which(c) for c in ("clang", "gcc", "cc")),
    reason="test requires a Modelica compiler and C compiler",
)


@pytest.mark.integration
@needs_backend_and_compiler
def test_two_fmu_federates_exchange_values():
    """Connect producer.y to consumer.u and verify the driven trajectory changes."""
    import helics

    backend = "rumoca" if shutil.which("rumoca") else "openmodelica"
    model = Model("Decay", [MODEL_FILE])
    with __import__("tempfile").TemporaryDirectory(prefix="polaris_helics_test_") as tmp:
        path = Path(tmp)
        producer_fmu = model.export_fmu(path / "producer.fmu", backend=backend)
        consumer_fmu = model.export_fmu(path / "consumer.fmu", backend=backend)
        broker_name = f"polaris-broker-{uuid4().hex[:8]}"
        broker = helics.helicsCreateBroker("zmq", broker_name, "--federates=2")
        assert broker is not None
        producer = FmuFederate(
            producer_fmu,
            "producer",
            outputs={"y": "producer.y"},
            step_size=0.05,
            broker=broker_name,
        )
        consumer = FmuFederate(
            consumer_fmu,
            "consumer",
            inputs={"u": "producer.y"},
            outputs={"y": "consumer.y"},
            step_size=0.05,
            broker=broker_name,
        )
        try:
            # Each HELICS federate waits for the other to join, so both must run concurrently.
            with ThreadPoolExecutor(max_workers=2) as pool:
                futures = [
                    pool.submit(producer.run, 0.5),
                    pool.submit(consumer.run, 0.5),
                ]
                producer_result, consumer_result = [future.result(timeout=60) for future in futures]
            assert producer_result.time == pytest.approx(consumer_result.time, abs=1e-12)
            assert consumer_result["x"][-1] > producer_result["x"][-1]
        finally:
            producer.close()
            consumer.close()
            helics.helicsBrokerWaitForDisconnect(broker, 5000)
            helics.helicsBrokerFree(broker)
