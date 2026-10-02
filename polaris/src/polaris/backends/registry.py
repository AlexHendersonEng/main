"""Backend registry with entry-point discovery.

Third-party packages register backends via the ``polaris.backends`` entry-point
group, e.g. in their pyproject.toml::

    [project.entry-points."polaris.backends"]
    mycompiler = "mypkg.backend:MyBackend"
"""

from __future__ import annotations

from collections.abc import Callable
from importlib.metadata import entry_points

from polaris.backends.base import Backend, BackendUnavailableError

ENTRY_POINT_GROUP = "polaris.backends"

BackendFactory = Callable[[], Backend]


# Built-ins are wrapped in functions so heavy imports happen only when the backend is used.
def _openmodelica() -> Backend:
    from polaris.backends.openmodelica import OpenModelicaBackend

    return OpenModelicaBackend()


_registry: dict[str, BackendFactory] = {"openmodelica": _openmodelica}
# Entry points are loaded lazily, once, so importing polaris stays fast.
_entry_points_loaded = False


def register_backend(name: str, factory: BackendFactory, *, replace: bool = False) -> None:
    """Register a factory (any zero-argument callable returning a Backend) under ``name``."""
    if name in _registry and not replace:
        raise ValueError(f"Backend '{name}' is already registered")
    _registry[name] = factory


def unregister_backend(name: str) -> None:
    _registry.pop(name, None)


def _load_entry_points() -> None:
    global _entry_points_loaded
    if _entry_points_loaded:
        return
    _entry_points_loaded = True
    for ep in entry_points(group=ENTRY_POINT_GROUP):
        # setdefault: explicit register_backend() calls win over entry points.
        _registry.setdefault(ep.name, ep.load())


def registered_backends() -> list[str]:
    _load_entry_points()
    return sorted(_registry)


def get_backend(name: str) -> Backend:
    _load_entry_points()
    try:
        factory = _registry[name]
    except KeyError:
        known = ", ".join(sorted(_registry)) or "none"
        raise KeyError(f"Unknown backend '{name}'. Registered: {known}") from None
    return factory()


def available_backends() -> list[str]:
    """Names of registered backends whose compiler is usable on this machine."""
    available = []
    for name in registered_backends():
        try:
            if get_backend(name).is_available():
                available.append(name)
        except BackendUnavailableError:
            # A backend that cannot even be constructed is simply not available.
            continue
    return available
