"""Apply a trained hit transformer to one frame of reconstructed hits.

No ``tid`` / ``hid`` / ``pid`` is used. Output is a cluster id per hit
(``-1`` = unassigned / noise).

tcsh, from this ``scitt/`` directory::

    python -m ml.infer --ckpt runs/best.pt --root /path/to/scitt.root
    python -m ml.infer --ckpt runs/best.pt --root /path/to/scitt.root --frame-id 42
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

_SCITT = Path(__file__).resolve().parents[1]
if str(_SCITT) not in sys.path:
    sys.path.insert(0, str(_SCITT))

import numpy as np
import torch

from ml.cluster import cluster_condensation
from ml.dataset import load_root_frames
from ml.features import FEATURE_DIM, encode_hits
from ml.model import HitTransformer, pick_device


def load_model(ckpt: str | Path, device: str = "auto") -> tuple[HitTransformer, torch.device, dict]:
    path = Path(ckpt)
    try:
        blob = torch.load(path, map_location="cpu", weights_only=False)
    except TypeError:
        blob = torch.load(path, map_location="cpu")
    args = blob.get("args") or {}
    feat_dim = int(blob.get("feature_dim", FEATURE_DIM))
    model = HitTransformer(
        in_dim=feat_dim,
        d_model=int(args.get("d_model", 64)),
        nhead=int(args.get("nhead", 4)),
        nlayers=int(args.get("nlayers", 2)),
        dim_ff=int(args.get("dim_ff", 128)),
        emb_dim=int(args.get("emb_dim", 8)),
        dropout=0.0,
    )
    model.load_state_dict(blob["model"])
    dev = pick_device(device)
    model.to(dev)
    model.eval()
    return model, dev, args


@torch.no_grad()
def cluster_hits(
    model: HitTransformer,
    hits: dict,
    device: torch.device | None = None,
    t_beta: float = 0.2,
    t_dist: float = 0.8,
) -> np.ndarray:
    """Return integer cluster labels ``[N]`` for one frame.

    Required hit arrays (length N): ``x, y, z, layer, tot, time``.
    ``r`` and ``phi`` are used if present, else computed from ``x,y``.
    Labels ``tid`` etc. are ignored.
    """
    if device is None:
        device = next(model.parameters()).device
    feats = encode_hits(hits)
    n = int(feats.shape[0])
    if n == 0:
        return np.zeros(0, dtype=np.int64)
    x = torch.from_numpy(feats).unsqueeze(0).to(device)
    mask = torch.ones(1, n, dtype=torch.bool, device=device)
    model.eval()
    emb, beta = model(x, mask)
    return cluster_condensation(emb[0], beta[0], mask[0], t_beta=t_beta, t_dist=t_dist)


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    p = argparse.ArgumentParser(description="cluster hits with a trained scitt model")
    p.add_argument("--ckpt", required=True, help="checkpoint from ml.train (best.pt / last.pt)")
    p.add_argument("--root", default="", help="scitt ROOT file; clusters the chosen frame")
    p.add_argument("--frame-id", type=int, default=-1, help="frame_id to run; default = first frame")
    p.add_argument("--device", default="auto")
    return p.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    model, device, _ = load_model(args.ckpt, device=args.device)
    if not args.root:
        print("loaded %s on %s" % (args.ckpt, device))
        print("pass --root FILE to cluster a dumped frame, or call ml.infer.cluster_hits(...)")
        return 0
    frames = load_root_frames(args.root)
    if not frames:
        raise SystemExit("no frames in %s" % args.root)
    if args.frame_id < 0:
        fr = frames[0]
    else:
        match = [f for f in frames if int(f["frame_id"]) == args.frame_id]
        if not match:
            raise SystemExit("frame_id %s not in %s" % (args.frame_id, args.root))
        fr = match[0]
    labels = cluster_hits(model, fr, device)
    n_cl = int(np.sum(np.unique(labels) >= 0))
    n_noise = int(np.sum(labels < 0))
    print("frame_id=%s  hits=%d  clusters=%d  unassigned=%d"
          % (fr["frame_id"], labels.size, n_cl, n_noise))
    print("labels:", " ".join(str(int(x)) for x in labels))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
