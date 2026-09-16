"""Hit features for the transformer. Labels (tid/hid/pid/mc_*) are never included."""

from __future__ import annotations

from typing import Any, Mapping

import numpy as np

# Mu3e 3-layer pixel radii / acceptance (mm, ns). Used only for scaling.
R_SCALE = 80.0
Z_SCALE = 200.0
TOT_SCALE = 1024.0
TIME_SCALE = 64.0  # frame length [ns]
N_LAYERS = 3

# Encoded vector: x,y,z,r, sinφ, cosφ, L0, L1, L2, tot, Δt  → 11
FEATURE_NAMES = (
    "x", "y", "z", "r",
    "sin_phi", "cos_phi",
    "layer0", "layer1", "layer2",
    "tot", "dt",
)
FEATURE_DIM = len(FEATURE_NAMES)

LABEL_COLUMNS = ("tid", "pid", "hid", "abs_hid", "n_mc", "mc_x", "mc_y", "mc_z")


def _arr(frame: Mapping[str, Any], key: str) -> np.ndarray:
    return np.asarray(frame[key])


def encode_hits(frame: Mapping[str, Any]) -> np.ndarray:
    """Return float32 features ``[N, FEATURE_DIM]`` from one frame.

    Time is centred on the frame median so an absolute Geant4 timestamp
    does not leak into the model. ``phi`` is encoded as sin/cos.
    """
    x = _arr(frame, "x").astype(np.float32, copy=False)
    n = x.shape[0]
    feats = np.zeros((n, FEATURE_DIM), dtype=np.float32)
    if n == 0:
        return feats

    y = _arr(frame, "y").astype(np.float32, copy=False)
    z = _arr(frame, "z").astype(np.float32, copy=False)
    r = _arr(frame, "r").astype(np.float32, copy=False)
    phi = _arr(frame, "phi").astype(np.float32, copy=False)
    layer = _arr(frame, "layer").astype(np.int64, copy=False)
    tot = _arr(frame, "tot").astype(np.float32, copy=False)
    time = _arr(frame, "time").astype(np.float32, copy=False)

    feats[:, 0] = x / R_SCALE
    feats[:, 1] = y / R_SCALE
    feats[:, 2] = z / Z_SCALE
    feats[:, 3] = r / R_SCALE
    feats[:, 4] = np.sin(phi)
    feats[:, 5] = np.cos(phi)
    for il in range(N_LAYERS):
        feats[:, 6 + il] = (layer == il).astype(np.float32)
    feats[:, 9] = np.clip(tot / TOT_SCALE, 0.0, 4.0)
    feats[:, 10] = (time - np.median(time)) / TIME_SCALE
    return feats


def assert_no_label_leak(names: tuple[str, ...] = FEATURE_NAMES) -> None:
    leaked = set(names) & set(LABEL_COLUMNS)
    if leaked:
        raise RuntimeError("label columns in model input: %s" % sorted(leaked))
