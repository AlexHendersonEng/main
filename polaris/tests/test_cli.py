"""Tests for Polaris command-line parsing and dispatch."""

from __future__ import annotations

import pytest

from polaris.backends import Backend, Capability, Version, register_backend, unregister_backend
from polaris.cli import main


class CliBackend(Backend):
    """Small backend for checking CLI argument forwarding without external tools."""

    name = "cli_test"
    capabilities = Capability.SIMULATE | Capability.EXPORT_FMU
    source_parameters: dict[str, float] = {}
    simulation_options = None

    def is_available(self) -> bool:
        return True

    def version(self) -> Version:
        return Version(1)

    def simulate(self, source, options, work_dir):
        type(self).source_parameters = source.parameters
        type(self).simulation_options = options
        path = work_dir / "result.csv"
        path.write_text("time,x\n0,1\n1,2\n", encoding="utf-8")
        return path


@pytest.fixture
def cli_backend():
    CliBackend.source_parameters = {}
    CliBackend.simulation_options = None
    register_backend("cli_test", CliBackend)
    yield
    unregister_backend("cli_test")


def test_simulate_dispatch_forwards_options_and_writes_csv(cli_backend, tmp_path, capsys):
    source = tmp_path / "model.mo"
    source.write_text("model M end M;", encoding="utf-8")
    destination = tmp_path / "nested" / "result.csv"
    status = main(
        [
            "simulate",
            "M",
            "--file",
            str(source),
            "--backend",
            "cli_test",
            "--parameter",
            "k=2.5",
            "--start",
            "0",
            "--stop",
            "3",
            "--step",
            "0.1",
            "--variable",
            "x",
            "--output",
            str(destination),
        ]
    )
    assert status == 0
    assert destination.is_file()
    assert "Saved 2 points" in capsys.readouterr().out
    assert CliBackend.source_parameters == {"k": 2.5}
    assert CliBackend.simulation_options.stop_time == 3.0
    assert CliBackend.simulation_options.outputs == ("x",)


@pytest.mark.parametrize(
    ("items", "expected"),
    [
        (["a=1", "b=-2.5"], {"a": 1.0, "b": -2.5}),
    ],
)
def test_cli_value_parsing(items, expected):
    from polaris.cli import _mapping

    assert _mapping(items) == expected


def test_cli_bounds_parsing():
    from polaris.cli import _bounds

    assert _bounds(["k=1,4"]) == {"k": (1.0, 4.0)}


@pytest.mark.parametrize("items", [["x"], ["x="], ["x=1", "x=2"], ["x=one"]])
def test_cli_rejects_bad_values(items):
    from polaris.cli import _mapping

    with pytest.raises(ValueError):
        _mapping(items)


@pytest.mark.parametrize("item", ["x=1", "x=2,1", "x=nan,2", "x=1,2,3"])
def test_cli_rejects_bad_bounds(item):
    from polaris.cli import _bounds

    with pytest.raises(ValueError):
        _bounds([item])


def test_invalid_parameter_is_reported(tmp_path, capsys, cli_backend):
    source = tmp_path / "model.mo"
    source.write_text("model M end M;", encoding="utf-8")
    status = main(
        [
            "simulate",
            "M",
            "--file",
            str(source),
            "--backend",
            "cli_test",
            "--parameter",
            "k=not-a-number",
            "--output",
            str(tmp_path / "out.csv"),
        ]
    )
    assert status == 1
    assert "polaris: error:" in capsys.readouterr().err
