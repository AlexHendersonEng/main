"""Unit tests for the declarative co-simulation config; no HELICS install required."""

from __future__ import annotations

import tomllib

import pytest

from polaris.cosim.config import CoSimulation, FederateSpec


def test_connections_derive_inputs_and_outputs():
    """A connection should surface as an output on the source and an input on the target."""
    config = CoSimulation(
        federates=[
            FederateSpec(name="producer", fmu="producer.fmu"),
            FederateSpec(name="consumer", fmu="consumer.fmu"),
        ],
        connections=[{"source": "producer.y", "target": "consumer.u"}],
        stop_time=1.0,
    )
    assert config.outputs_for("producer") == {"y": "producer.y"}
    assert config.inputs_for("producer") == {}
    assert config.inputs_for("consumer") == {"u": "producer.y"}
    assert config.outputs_for("consumer") == {}


def test_duplicate_federate_names_rejected():
    with pytest.raises(ValueError, match="unique"):
        CoSimulation(
            federates=[
                FederateSpec(name="a", fmu="a.fmu"),
                FederateSpec(name="a", fmu="b.fmu"),
            ],
            stop_time=1.0,
        )


def test_connection_referencing_unknown_federate_rejected():
    with pytest.raises(ValueError, match="not in"):
        CoSimulation(
            federates=[FederateSpec(name="a", fmu="a.fmu")],
            connections=[{"source": "a.y", "target": "missing.u"}],
            stop_time=1.0,
        )


def test_connection_requires_dotted_reference():
    with pytest.raises(ValueError, match="federate.variable"):
        CoSimulation(
            federates=[FederateSpec(name="a", fmu="a.fmu")],
            connections=[{"source": "a.y", "target": "no-dot-here"}],
            stop_time=1.0,
        )


def test_federate_name_rejects_empty_and_dotted():
    with pytest.raises(ValueError, match="empty"):
        FederateSpec(name="", fmu="a.fmu")
    with pytest.raises(ValueError, match="must not contain"):
        FederateSpec(name="a.b", fmu="a.fmu")


def test_stop_time_must_be_positive():
    with pytest.raises(ValueError, match="stop_time"):
        CoSimulation(federates=[FederateSpec(name="a", fmu="a.fmu")], stop_time=0.0)


def test_from_dict_builds_specs_and_connections():
    config = CoSimulation.from_dict(
        {
            "stop_time": 2.0,
            "broker_name": "fixed-broker",
            "federates": [
                {"name": "producer", "fmu": "producer.fmu", "step_size": 0.05},
                {"name": "consumer", "fmu": "consumer.fmu", "step_size": 0.05},
            ],
            "connections": [{"source": "producer.y", "target": "consumer.u"}],
        }
    )
    assert config.stop_time == 2.0
    assert config.broker_name == "fixed-broker"
    assert [f.name for f in config.federates] == ["producer", "consumer"]
    assert config.inputs_for("consumer") == {"u": "producer.y"}


def test_from_toml_round_trips_through_from_dict(tmp_path):
    toml_text = """
    stop_time = 1.5

    [[federates]]
    name = "producer"
    fmu = "producer.fmu"
    step_size = 0.1

    [[federates]]
    name = "consumer"
    fmu = "consumer.fmu"
    step_size = 0.1

    [[connections]]
    source = "producer.y"
    target = "consumer.u"
    """
    toml_path = tmp_path / "cosim.toml"
    toml_path.write_text(toml_text)
    config = CoSimulation.from_toml(toml_path)
    assert config.stop_time == 1.5
    assert config.inputs_for("consumer") == {"u": "producer.y"}
    # Sanity-check the fixture itself parses the way from_toml expects.
    assert tomllib.loads(toml_text)["federates"][0]["name"] == "producer"
