"""Dumb baseline: DBSCAN on (sin φ, cos φ, z, t). No learned embedding."""

from __future__ import annotations

from typing import Any

import numpy as np
from sklearn.cluster import DBSCAN

from ml.features import TIME_SCALE, Z_SCALE


def dbscan_phi_z_t(
    frame: dict[str, Any],
    eps: float = 0.25,
    min_samples: int = 2,
) -> np.ndarray:
    """Cluster reconstructed hits in φ–z–t. Recurls with a large Δz often split."""
    n = int(np.asarray(frame["x"]).shape[0])
    if n == 0:
        return np.zeros(0, dtype=np.int64)
    phi = np.asarray(frame["phi"], dtype=np.float32)
    z = np.asarray(frame["z"], dtype=np.float32)
    t = np.asarray(frame["time"], dtype=np.float32)
    dt = (t - np.median(t)) / TIME_SCALE
    feats = np.stack(
        [np.sin(phi), np.cos(phi), z / Z_SCALE, dt],
        axis=1,
    )
    if n < min_samples:
        return np.full(n, -1, dtype=np.int64)
    pred = DBSCAN(eps=eps, min_samples=min_samples).fit_predict(feats)
    return pred.astype(np.int64)
