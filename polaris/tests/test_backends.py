import pytest

from polaris.backends import (
    Backend,
    Capability,
    UnsupportedCapabilityError,
    Version,
    VersionAdapter,
    available_backends,
    get_backend,
    register_backend,
    registered_backends,
    select_adapter,
    unregister_backend,
)
from polaris.types import FmuKind, ModelSource, SimulationOptions


class FakeBackend(Backend):
    name = "fake"
    capabilities = Capability.SIMULATE

    def __init__(self, available: bool = True) -> None:
        self._available = available

    def is_available(self) -> bool:
        return self._available

    def version(self) -> Version:
        return Version(1, 2, 3)


@pytest.fixture
def fake():
    register_backend("fake", FakeBackend)
    register_backend("ghost", lambda: FakeBackend(available=False))
    yield
    unregister_backend("fake")
    unregister_backend("ghost")


def test_registry(fake):
    assert {"fake", "ghost"} <= set(registered_backends())
    assert "fake" in available_backends()
    assert "ghost" not in available_backends()
    assert get_backend("fake").supports(Capability.SIMULATE)


def test_duplicate_and_unknown(fake):
    with pytest.raises(ValueError):
        register_backend("fake", FakeBackend)
    with pytest.raises(KeyError):
        get_backend("nope")


def test_unsupported_capability(fake, tmp_path):
    backend = get_backend("fake")
    assert not backend.supports(Capability.EXPORT_FMU)
    with pytest.raises(UnsupportedCapabilityError):
        backend.export_fmu(ModelSource("M"), FmuKind.CO_SIMULATION, "2.0", tmp_path)
    assert backend.simulate.__name__ == "simulate"
    assert SimulationOptions().stop_time == 1.0


def test_version_parse_and_order():
    assert Version.parse("OpenModelica v1.27.1 (64-bit)") == Version(1, 27, 1)
    assert Version.parse("rumoca 0.10.0") > Version(0, 9, 9)
    assert Version.parse("2") == Version(2, 0, 0)
    with pytest.raises(ValueError):
        Version.parse("none")


def test_select_adapter():
    adapters = [
        VersionAdapter(Version(0, 1), {"flag": "old"}),
        VersionAdapter(Version(0, 10), {"flag": "new"}),
    ]
    assert select_adapter(Version(0, 9, 5), adapters).settings["flag"] == "old"
    assert select_adapter(Version(0, 10, 0), adapters).settings["flag"] == "new"
    assert select_adapter(Version(1, 0), adapters).settings["flag"] == "new"
    with pytest.raises(ValueError):
        select_adapter(Version(0, 0, 1), adapters)
