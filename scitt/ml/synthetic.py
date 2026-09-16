"""Toy 3-layer frames so the pipeline can run without a ROOT dump.

Hits of one ``tid`` share a helical (φ, z, t) pattern, including optional
L3 recurls. Geometry numbers match Mu3e order-of-magnitude, not a full sim.
"""

from __future__ import annotations

from typing import Any

import numpy as np

from ml.dataset import FrameDataset

# 3-layer radii [mm], 0-based layer index = L1, L2, L3.
LAYER_R = np.array([23.5, 29.8, 72.0], dtype=np.float32)


def generate_frame(
    frame_id: int,
    rng: np.random.Generator,
    n_tracks: tuple[int, int] = (6, 14),
    n_noise: tuple[int, int] = (4, 16),
    recurl_prob: float = 0.35,
    miss_layer_prob: float = 0.08,
) -> dict[str, Any]:
    n_tr = int(rng.integers(n_tracks[0], n_tracks[1] + 1))
    chunks: list[dict[str, np.ndarray]] = []
    hid_base = 1
    for itrack in range(n_tr):
        tid = itrack + 1
        pid = 11 if rng.random() < 0.5 else -11
        q = 1.0 if pid == 11 else -1.0
        phi0 = float(rng.uniform(-np.pi, np.pi))
        z0 = float(rng.uniform(-40.0, 40.0))
        t0 = float(rng.uniform(0.0, 50.0))
        # pT ~ 25–50 MeV → helix radius tens to ~150 mm
        pt = float(rng.uniform(25.0, 50.0))
        r_helix = pt / 0.3
        tanl = float(rng.uniform(-0.8, 0.8))
        miss = {
            il: rng.random() < miss_layer_prob
            for il in range(3)
        }
        if sum(1 for v in miss.values() if not v) < 2:
            miss[0] = False
            miss[2] = False

        xs, ys, zs, rs, phis, layers, tots, times = [], [], [], [], [], [], [], []
        hids, tids, pids, sensors, nmcs = [], [], [], [], []
        hid = hid_base
        for il, r in enumerate(LAYER_R):
            if miss[il]:
                hid += 1
                continue
            # simple cylinder-crossing: φ advances with arc r / R
            dphi = q * (r / r_helix)
            phi = phi0 + dphi
            z = z0 + tanl * r
            path = np.hypot(r, z - z0)
            t = t0 + path / 299.792458  # mm / (mm/ns) ≈ ns
            x = r * np.cos(phi)
            y = r * np.sin(phi)
            xs.append(x + rng.normal(0.0, 0.02))
            ys.append(y + rng.normal(0.0, 0.02))
            zs.append(z + rng.normal(0.0, 0.08))
            rs.append(np.hypot(xs[-1], ys[-1]))
            phis.append(np.arctan2(ys[-1], xs[-1]))
            layers.append(il)
            tots.append(int(rng.integers(300, 900)))
            times.append(t + rng.normal(0.0, 0.05))
            hids.append(hid)
            hid += 1
            tids.append(tid)
            pids.append(pid)
            sensors.append(il * 1000 + int(((phi + np.pi) / (2 * np.pi)) * 12) % 12)
            nmcs.append(1)

        if rng.random() < recurl_prob and not miss[2]:
            r = float(LAYER_R[2])
            phi = phi0 + q * (r / r_helix) + q * np.pi * 0.7
            z = z0 + tanl * r + rng.choice([-1.0, 1.0]) * rng.uniform(80.0, 140.0)
            t = t0 + 2.5 + rng.normal(0.0, 0.2)
            x = r * np.cos(phi)
            y = r * np.sin(phi)
            xs.append(x + rng.normal(0.0, 0.02))
            ys.append(y + rng.normal(0.0, 0.02))
            zs.append(z + rng.normal(0.0, 0.08))
            rs.append(np.hypot(xs[-1], ys[-1]))
            phis.append(np.arctan2(ys[-1], xs[-1]))
            layers.append(2)
            tots.append(int(rng.integers(300, 900)))
            times.append(t)
            hids.append(-hid)
            tids.append(tid)
            pids.append(pid)
            sensors.append(2000 + int(((phi + np.pi) / (2 * np.pi)) * 12) % 12)
            nmcs.append(1)

        if xs:
            chunks.append({
                "x": np.array(xs, dtype=np.float32),
                "y": np.array(ys, dtype=np.float32),
                "z": np.array(zs, dtype=np.float32),
                "r": np.array(rs, dtype=np.float32),
                "phi": np.array(phis, dtype=np.float32),
                "layer": np.array(layers, dtype=np.int64),
                "tot": np.array(tots, dtype=np.int64),
                "time": np.array(times, dtype=np.float32),
                "tid": np.array(tids, dtype=np.int64),
                "pid": np.array(pids, dtype=np.int64),
                "hid": np.array(hids, dtype=np.int64),
                "sensor_id": np.array(sensors, dtype=np.int64),
                "n_mc": np.array(nmcs, dtype=np.int64),
            })

    n_n = int(rng.integers(n_noise[0], n_noise[1] + 1))
    if n_n > 0:
        layer_n = rng.integers(0, 3, size=n_n)
        r_n = LAYER_R[layer_n] + rng.normal(0.0, 0.15, size=n_n)
        phi_n = rng.uniform(-np.pi, np.pi, size=n_n)
        chunks.append({
            "x": (r_n * np.cos(phi_n)).astype(np.float32),
            "y": (r_n * np.sin(phi_n)).astype(np.float32),
            "z": rng.uniform(-180.0, 180.0, size=n_n).astype(np.float32),
            "r": r_n.astype(np.float32),
            "phi": phi_n.astype(np.float32),
            "layer": layer_n.astype(np.int64),
            "tot": rng.integers(200, 800, size=n_n).astype(np.int64),
            "time": rng.uniform(-5.0, 55.0, size=n_n).astype(np.float32),
            "tid": np.zeros(n_n, dtype=np.int64),
            "pid": np.zeros(n_n, dtype=np.int64),
            "hid": np.zeros(n_n, dtype=np.int64),
            "sensor_id": (layer_n * 1000 + rng.integers(0, 12, size=n_n)).astype(np.int64),
            "n_mc": np.zeros(n_n, dtype=np.int64),
        })

    frame: dict[str, Any] = {"frame_id": int(frame_id)}
    if not chunks:
        from ml.dataset import empty_frame
        return empty_frame(frame_id)
    for key in chunks[0]:
        frame[key] = np.concatenate([c[key] for c in chunks])
    return frame


def make_synthetic_dataset(
    n_frames: int,
    seed: int = 0,
    **kwargs: Any,
) -> FrameDataset:
    rng = np.random.default_rng(seed)
    frames = [generate_frame(i, rng, **kwargs) for i in range(n_frames)]
    return FrameDataset(frames)
