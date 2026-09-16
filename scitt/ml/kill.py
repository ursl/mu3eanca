"""Simulate dead sensors / chips / layers by dropping hits.

The drop is the experiment: association should survive missing 'next' hits.
``sensor`` and ``chip`` are the same operation (``sensor_id`` is the chip).
"""

from __future__ import annotations

from typing import Any

import numpy as np

from ml.dataset import n_hits, slice_frame

KILL_MODES = ("none", "hit", "sensor", "chip", "layer")


def kill_hits(
    frame: dict[str, Any],
    rng: np.random.Generator,
    mode: str = "none",
    fraction: float = 0.0,
) -> dict[str, Any]:
    """Return a copy of ``frame`` with some hits removed.

    * ``hit``: each hit dropped independently with probability ``fraction``.
    * ``sensor`` / ``chip``: each ``sensor_id`` dropped independently.
    * ``layer``: each layer dropped independently.
    * ``none``: identity.

    If every hit would be removed, the original frame is kept.
    """
    if mode in ("none", "", None) or fraction <= 0.0:
        return frame
    if mode == "chip":
        mode = "sensor"
    if mode not in ("hit", "sensor", "layer"):
        raise ValueError("unknown kill mode %r (want %s)" % (mode, KILL_MODES))

    n = n_hits(frame)
    if n == 0:
        return frame

    if mode == "hit":
        keep = rng.random(n) >= fraction
    elif mode == "sensor":
        sensors = np.asarray(frame["sensor_id"])
        uniq = np.unique(sensors)
        drop = uniq[rng.random(uniq.size) < fraction]
        keep = ~np.isin(sensors, drop)
    else:
        layers = np.asarray(frame["layer"])
        uniq = np.unique(layers)
        drop = uniq[rng.random(uniq.size) < fraction]
        keep = ~np.isin(layers, drop)

    if not bool(np.any(keep)):
        return frame
    return slice_frame(frame, keep)


def kill_fraction_actual(original: dict[str, Any], killed: dict[str, Any]) -> float:
    n0 = n_hits(original)
    if n0 == 0:
        return 0.0
    return 1.0 - n_hits(killed) / float(n0)


def make_kill_collate(
    mode: str,
    fraction: float,
    training: bool,
    seed: int = 0,
):
    """DataLoader collate: optional random kill, then encode and pad."""
    from ml.dataset import collate_encoded, encode_frame

    rng = np.random.default_rng(seed)

    def _collate(frames: list[dict[str, Any]]) -> dict[str, Any]:
        encoded = []
        for fr in frames:
            frac = fraction
            if training and fraction > 0.0 and mode not in ("none", "", None):
                frac = float(rng.uniform(0.0, fraction))
            killed = kill_hits(fr, rng, mode=mode or "none", fraction=frac)
            encoded.append(encode_frame(killed))
        return collate_encoded(encoded)

    return _collate
