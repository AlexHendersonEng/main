"""Jacobians of a model's state derivatives."""

from __future__ import annotations

import tempfile
from collections.abc import Mapping
from pathlib import Path

from polaris.backends import (
    Backend,
    Capability,
    UnsupportedCapabilityError,
    available_backends,
    get_backend,
)
from polaris.model import Model
from polaris.types import Jacobian


def jacobian(
    model: Model,
    at: Mapping[str, float] | None = None,
    *,
    backend: str | Backend | None = None,
    work_dir: str | Path | None = None,
) -> Jacobian:
    """Jacobian of the state derivatives with respect to the states and the parameters.

    Args:
        model: The model to differentiate (its default parameters are applied).
        at: State values to evaluate at; unlisted states keep their initial value.
        backend: Backend to use. When omitted (and the model names none), the first
            installed backend with native Jacobian support is chosen.
        work_dir: Keep build artefacts here instead of a temporary directory.

    Raises:
        UnsupportedCapabilityError: the chosen backend, or every installed backend, has no
            native Jacobian. Use :func:`polaris.analysis.local_sensitivity` for a
            backend-independent finite-difference alternative with respect to parameters.
    """
    chosen = backend or model.backend
    if isinstance(chosen, Backend):
        be = chosen
    elif chosen is not None:
        be = get_backend(chosen)
    else:
        # Auto-selection must skip installed backends that cannot differentiate.
        candidates = [get_backend(n) for n in available_backends()]
        capable = [b for b in candidates if b.supports(Capability.JACOBIAN)]
        if not capable:
            raise UnsupportedCapabilityError(
                "No installed backend provides native Jacobians; "
                "use polaris.analysis.local_sensitivity instead"
            )
        be = capable[0]
    if not be.supports(Capability.JACOBIAN):
        raise UnsupportedCapabilityError(
            f"Backend '{be.name}' has no native Jacobian; "
            "use polaris.analysis.local_sensitivity instead"
        )
    if work_dir is not None:
        return be.jacobian(model.source(), Path(work_dir), at)
    with tempfile.TemporaryDirectory(prefix="polaris_") as tmp:
        return be.jacobian(model.source(), Path(tmp), at)
