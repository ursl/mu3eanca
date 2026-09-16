"""Per-frame hit containers, ROOT loader, padding collate."""

from __future__ import annotations

from typing import Any, Iterable, Optional, Sequence

import numpy as np

from ml.features import encode_hits

HIT_COLUMNS = (
    "x", "y", "z", "r", "phi", "layer", "tot", "time",
    "tid", "pid", "hid", "sensor_id", "n_mc",
)


def empty_frame(frame_id: int = 0) -> dict[str, Any]:
    z32 = np.zeros(0, dtype=np.float32)
    z64 = np.zeros(0, dtype=np.int64)
    return {
        "frame_id": int(frame_id),
        "x": z32, "y": z32, "z": z32, "r": z32, "phi": z32, "time": z32,
        "layer": z64, "tot": z64, "tid": z64, "pid": z64, "hid": z64,
        "sensor_id": z64, "n_mc": z64,
    }


def n_hits(frame: dict[str, Any]) -> int:
    return int(np.asarray(frame["x"]).shape[0])


def slice_frame(frame: dict[str, Any], keep: np.ndarray) -> dict[str, Any]:
    out = {"frame_id": frame["frame_id"]}
    for key in HIT_COLUMNS:
        out[key] = np.asarray(frame[key])[keep]
    return out


def _col(frame: dict[str, Any], key: str, n: int, dtype, default=0):
    if key not in frame:
        return np.full(n, default, dtype=dtype)
    return np.asarray(frame[key], dtype=dtype)


def encode_frame(frame: dict[str, Any]) -> dict[str, Any]:
    feats = encode_hits(frame)
    n = int(feats.shape[0])
    return {
        "features": feats,
        "tid": _col(frame, "tid", n, np.int64),
        "pid": _col(frame, "pid", n, np.int64),
        "layer": _col(frame, "layer", n, np.int64, -1),
        "sensor_id": _col(frame, "sensor_id", n, np.int64),
        "hid": _col(frame, "hid", n, np.int64),
        "n_mc": _col(frame, "n_mc", n, np.int64),
        "phi": _col(frame, "phi", n, np.float32),
        "z": _col(frame, "z", n, np.float32),
        "time": _col(frame, "time", n, np.float32),
        "frame_id": int(frame.get("frame_id", 0)),
    }


def group_hits_by_frame(cols: dict[str, np.ndarray]) -> list[dict[str, Any]]:
    fid = np.asarray(cols["frame_id"])
    if fid.size == 0:
        return []
    order = np.argsort(fid, kind="mergesort")
    fid_sorted = fid[order]
    cuts = np.flatnonzero(fid_sorted[1:] != fid_sorted[:-1]) + 1
    bounds = np.concatenate(([0], cuts, [fid_sorted.size]))
    frames: list[dict[str, Any]] = []
    for a, b in zip(bounds[:-1], bounds[1:]):
        idx = order[a:b]
        fr = {"frame_id": int(fid_sorted[a])}
        for key in HIT_COLUMNS:
            if key not in cols:
                raise KeyError("hits tree missing branch %s" % key)
            fr[key] = np.asarray(cols[key])[idx]
        frames.append(fr)
    return frames


def load_root_frames(path: str, max_frames: Optional[int] = None) -> list[dict[str, Any]]:
    """Load the flat ``hits`` tree and group rows by ``frame_id``."""
    try:
        import uproot
    except ImportError as exc:
        import sys
        raise ImportError(
            "reading a ROOT dump needs uproot in this Python:\n"
            "  %s\n"
            "Install with:  python -m pip install uproot\n"
            "(bare `pip install` can target a different interpreter)"
            % sys.executable
        ) from exc

    branches = ("frame_id",) + HIT_COLUMNS
    with uproot.open(path) as f:
        if "hits" not in f:
            raise KeyError("%s has no 'hits' tree" % path)
        raw = f["hits"].arrays(branches, library="np")
    frames = group_hits_by_frame(raw)
    if max_frames is not None:
        frames = frames[: max_frames]
    return frames


class FrameDataset:
    """One item = one frame of reconstructed hits (numpy, not yet padded)."""

    def __init__(self, frames: Sequence[dict[str, Any]], min_hits: int = 1):
        self.frames = [fr for fr in frames if n_hits(fr) >= min_hits]
        if not self.frames:
            raise ValueError("no frames with at least %d hits" % min_hits)

    def __len__(self) -> int:
        return len(self.frames)

    def __getitem__(self, idx: int) -> dict[str, Any]:
        return self.frames[idx]


def collate_encoded(items: Iterable[dict[str, Any]]) -> dict[str, Any]:
    """Pad variable-N frames to a batch. ``hit_mask`` is True for real hits."""
    import torch

    encoded = [it if "features" in it else encode_frame(it) for it in items]
    if not encoded:
        raise ValueError("empty batch")
    bsz = len(encoded)
    nmax = max(int(e["features"].shape[0]) for e in encoded)
    nmax = max(nmax, 1)
    feat_dim = int(encoded[0]["features"].shape[1])

    features = np.zeros((bsz, nmax, feat_dim), dtype=np.float32)
    hit_mask = np.zeros((bsz, nmax), dtype=np.bool_)
    tid = np.zeros((bsz, nmax), dtype=np.int64)
    pid = np.zeros((bsz, nmax), dtype=np.int64)
    layer = np.full((bsz, nmax), -1, dtype=np.int64)
    sensor_id = np.zeros((bsz, nmax), dtype=np.int64)
    hid = np.zeros((bsz, nmax), dtype=np.int64)
    n_mc = np.zeros((bsz, nmax), dtype=np.int64)
    phi = np.zeros((bsz, nmax), dtype=np.float32)
    z = np.zeros((bsz, nmax), dtype=np.float32)
    time = np.zeros((bsz, nmax), dtype=np.float32)
    frame_ids = np.zeros(bsz, dtype=np.int64)

    for i, e in enumerate(encoded):
        n = int(e["features"].shape[0])
        if n == 0:
            continue
        features[i, :n] = e["features"]
        hit_mask[i, :n] = True
        tid[i, :n] = e["tid"]
        pid[i, :n] = e["pid"]
        layer[i, :n] = e["layer"]
        sensor_id[i, :n] = e["sensor_id"]
        hid[i, :n] = e["hid"]
        n_mc[i, :n] = e["n_mc"]
        phi[i, :n] = e["phi"]
        z[i, :n] = e["z"]
        time[i, :n] = e["time"]
        frame_ids[i] = e["frame_id"]

    return {
        "features": torch.from_numpy(features),
        "hit_mask": torch.from_numpy(hit_mask),
        "tid": torch.from_numpy(tid),
        "pid": torch.from_numpy(pid),
        "layer": torch.from_numpy(layer),
        "sensor_id": torch.from_numpy(sensor_id),
        "hid": torch.from_numpy(hid),
        "n_mc": torch.from_numpy(n_mc),
        "phi": torch.from_numpy(phi),
        "z": torch.from_numpy(z),
        "time": torch.from_numpy(time),
        "frame_id": torch.from_numpy(frame_ids),
    }
