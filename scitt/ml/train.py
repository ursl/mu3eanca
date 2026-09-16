"""Train / evaluate the hit transformer.

tcsh examples (from this directory, ``scitt/``)::

    python -m ml.train --synthetic --epochs 8 --kill-mode sensor --kill-frac 0.3
    python -m ml.train --root /path/to/scitt.root --epochs 20 --eval-fracs 0,0.1,0.2,0.4
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
from torch.utils.data import DataLoader

from ml.baseline import dbscan_phi_z_t
from ml.cluster import cluster_condensation
from ml.dataset import FrameDataset, encode_frame, load_root_frames, n_hits
from ml.features import FEATURE_DIM, assert_no_label_leak
from ml.kill import kill_fraction_actual, kill_hits, make_kill_collate
from ml.loss import condensation_loss, contrastive_loss
from ml.metrics import MetricAccum, format_metrics
from ml.model import HitTransformer, pick_device
from ml.synthetic import make_synthetic_dataset


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    p = argparse.ArgumentParser(description="scitt hit-transformer track finding")
    src = p.add_mutually_exclusive_group()
    src.add_argument("--root", default="", help="scitt ROOT file with a hits tree")
    src.add_argument("--synthetic", action="store_true", help="toy 3-layer frames")
    p.add_argument("--n-train", type=int, default=128)
    p.add_argument("--n-val", type=int, default=32)
    p.add_argument("--max-frames", type=int, default=0, help="cap ROOT frames (0 = all)")
    p.add_argument("--val-frac", type=float, default=0.2, help="ROOT split if no synthetic")
    p.add_argument("--epochs", type=int, default=10)
    p.add_argument("--batch-size", type=int, default=4)
    p.add_argument("--lr", type=float, default=1e-3)
    p.add_argument("--d-model", type=int, default=64)
    p.add_argument("--nhead", type=int, default=4)
    p.add_argument("--nlayers", type=int, default=2)
    p.add_argument("--dim-ff", type=int, default=128)
    p.add_argument("--emb-dim", type=int, default=8)
    p.add_argument("--dropout", type=float, default=0.1)
    p.add_argument("--loss", choices=("oc", "contrastive"), default="oc")
    p.add_argument("--kill-mode", choices=("none", "hit", "sensor", "chip", "layer"), default="sensor")
    p.add_argument("--kill-frac", type=float, default=0.25, help="max drop probability in training")
    p.add_argument("--eval-fracs", default="0,0.15,0.3,0.5", help="comma-separated drop scan")
    p.add_argument("--min-hits", type=int, default=2, help="reconstructable after drop")
    p.add_argument("--seed", type=int, default=0)
    p.add_argument("--device", default="auto")
    p.add_argument("--out", default="", help="checkpoint directory (default runs/)")
    p.add_argument("--no-baseline", action="store_true")
    p.add_argument("--fast", action="store_true", help="tiny model / few frames")
    return p.parse_args(argv)


def _split_root(frames: list, val_frac: float, seed: int) -> tuple[FrameDataset, FrameDataset]:
    rng = np.random.default_rng(seed)
    idx = rng.permutation(len(frames))
    n_val = max(1, int(round(len(frames) * val_frac)))
    val_i = set(idx[:n_val].tolist())
    train = [frames[i] for i in range(len(frames)) if i not in val_i]
    val = [frames[i] for i in range(len(frames)) if i in val_i]
    if not train:
        train = frames
    if not val:
        val = frames[-max(1, len(frames) // 5):]
    return FrameDataset(train), FrameDataset(val)


def load_datasets(args: argparse.Namespace) -> tuple[FrameDataset, FrameDataset]:
    if args.fast:
        args.n_train = min(args.n_train, 32)
        args.n_val = min(args.n_val, 8)
        args.d_model = min(args.d_model, 32)
        args.dim_ff = min(args.dim_ff, 64)
        args.epochs = min(args.epochs, 3)
        args.batch_size = min(args.batch_size, 4)
    if args.root:
        cap = args.max_frames or None
        frames = load_root_frames(args.root, max_frames=cap)
        return _split_root(frames, args.val_frac, args.seed)
    if not args.synthetic and not args.root:
        args.synthetic = True
    train = make_synthetic_dataset(args.n_train, seed=args.seed)
    val = make_synthetic_dataset(args.n_val, seed=args.seed + 10_000)
    return train, val


def batch_to_device(batch: dict, device: torch.device) -> dict:
    out = {}
    for k, v in batch.items():
        out[k] = v.to(device) if torch.is_tensor(v) else v
    return out


def train_one_epoch(model, loader, opt, device, loss_name: str) -> dict[str, float]:
    model.train()
    totals: dict[str, float] = {}
    n = 0
    for batch in loader:
        batch = batch_to_device(batch, device)
        opt.zero_grad(set_to_none=True)
        emb, beta = model(batch["features"], batch["hit_mask"])
        if loss_name == "oc":
            loss, stats = condensation_loss(emb, beta, batch["tid"], batch["hit_mask"])
        else:
            loss, stats = contrastive_loss(emb, batch["tid"], batch["hit_mask"])
        if not torch.isfinite(loss):
            continue
        loss.backward()
        torch.nn.utils.clip_grad_norm_(model.parameters(), 5.0)
        opt.step()
        n += 1
        for k, v in stats.items():
            totals[k] = totals.get(k, 0.0) + v
    if n == 0:
        return {"loss": float("nan")}
    return {k: v / n for k, v in totals.items()}


@torch.no_grad()
def predict_frame(model, frame: dict, device: torch.device) -> np.ndarray:
    enc = encode_frame(frame)
    feats = torch.from_numpy(enc["features"]).unsqueeze(0).to(device)
    mask = torch.ones(1, feats.shape[1], dtype=torch.bool, device=device)
    if feats.shape[1] == 0:
        return np.zeros(0, dtype=np.int64)
    model.eval()
    emb, beta = model(feats, mask)
    return cluster_condensation(emb[0], beta[0], mask[0])


def evaluate(
    frames: FrameDataset,
    *,
    model=None,
    device: torch.device | None = None,
    kill_mode: str = "none",
    kill_frac: float = 0.0,
    seed: int = 0,
    min_hits: int = 2,
    use_baseline: bool = False,
) -> dict[str, float]:
    rng = np.random.default_rng(seed)
    acc = MetricAccum()
    for i in range(len(frames)):
        raw = frames[i]
        killed = kill_hits(raw, rng, mode=kill_mode, fraction=kill_frac)
        if n_hits(killed) == 0:
            continue
        if use_baseline:
            pred = dbscan_phi_z_t(killed)
        else:
            assert model is not None and device is not None
            pred = predict_frame(model, killed, device)
        acc.add(
            killed["tid"],
            pred,
            hid=killed["hid"],
            min_hits=min_hits,
            kill_frac=kill_fraction_actual(raw, killed),
        )
    return acc.as_dict()


def drop_scan(
    frames: FrameDataset,
    fracs: list[float],
    *,
    model=None,
    device=None,
    kill_mode: str,
    seed: int,
    min_hits: int,
    with_baseline: bool,
) -> list[tuple[float, dict[str, float], dict[str, float] | None]]:
    rows = []
    for frac in fracs:
        nn_m = evaluate(
            frames, model=model, device=device,
            kill_mode=kill_mode if frac > 0 else "none",
            kill_frac=frac, seed=seed, min_hits=min_hits, use_baseline=False,
        ) if model is not None else None
        bl_m = evaluate(
            frames, kill_mode=kill_mode if frac > 0 else "none",
            kill_frac=frac, seed=seed, min_hits=min_hits, use_baseline=True,
        ) if with_baseline else None
        rows.append((frac, nn_m or {}, bl_m))
    return rows


def save_ckpt(path: Path, model: HitTransformer, args: argparse.Namespace) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    torch.save(
        {
            "model": model.state_dict(),
            "args": vars(args),
            "feature_dim": FEATURE_DIM,
        },
        path,
    )


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    assert_no_label_leak()
    torch.manual_seed(args.seed)
    np.random.seed(args.seed)

    device = pick_device(args.device)
    train_ds, val_ds = load_datasets(args)
    print("device=%s  train_frames=%d  val_frames=%d  feat_dim=%d"
          % (device, len(train_ds), len(val_ds), FEATURE_DIM))

    collate = make_kill_collate(
        mode=args.kill_mode,
        fraction=args.kill_frac,
        training=True,
        seed=args.seed,
    )
    loader = DataLoader(
        train_ds,
        batch_size=args.batch_size,
        shuffle=True,
        collate_fn=collate,
        num_workers=0,
    )

    model = HitTransformer(
        in_dim=FEATURE_DIM,
        d_model=args.d_model,
        nhead=args.nhead,
        nlayers=args.nlayers,
        dim_ff=args.dim_ff,
        emb_dim=args.emb_dim,
        dropout=args.dropout,
    ).to(device)
    opt = torch.optim.AdamW(model.parameters(), lr=args.lr)

    out_dir = Path(args.out) if args.out else Path(__file__).resolve().parents[1] / "runs"
    best_eff = -1.0
    for epoch in range(1, args.epochs + 1):
        stats = train_one_epoch(model, loader, opt, device, args.loss)
        val_m = evaluate(
            val_ds, model=model, device=device,
            kill_mode="none", kill_frac=0.0, seed=args.seed + epoch,
            min_hits=args.min_hits,
        )
        drop_m = evaluate(
            val_ds, model=model, device=device,
            kill_mode=args.kill_mode, kill_frac=args.kill_frac,
            seed=args.seed + epoch, min_hits=args.min_hits,
        )
        print("epoch %03d  loss=%.4f  att=%.3f rep=%.3f beta=%.3f noise=%.3f"
              % (epoch, stats.get("loss", float("nan")),
                 stats.get("att", 0.0), stats.get("rep", 0.0),
                 stats.get("beta", 0.0), stats.get("noise", 0.0)))
        print("  " + format_metrics("val", val_m))
        print("  " + format_metrics("val@kill=%.2f" % args.kill_frac, drop_m))
        if val_m["efficiency"] >= best_eff:
            best_eff = val_m["efficiency"]
            save_ckpt(out_dir / "best.pt", model, args)

    save_ckpt(out_dir / "last.pt", model, args)

    fracs = [float(x) for x in args.eval_fracs.split(",") if x.strip() != ""]
    scan_mode = args.kill_mode if args.kill_mode != "none" else "sensor"
    print("drop scan  mode=%s  (reconstructable if >= %d hits remain)"
          % (scan_mode, args.min_hits))
    print("%8s  %8s %8s %8s  %8s %8s %8s"
          % ("frac", "nn_eff", "nn_fake", "nn_recurl", "db_eff", "db_fake", "db_recurl"))
    rows = drop_scan(
        val_ds, fracs, model=model, device=device,
        kill_mode=scan_mode, seed=args.seed + 999,
        min_hits=args.min_hits, with_baseline=not args.no_baseline,
    )
    for frac, nn_m, bl_m in rows:
        bl = bl_m or {"efficiency": 0.0, "fake_rate": 0.0, "recurl_efficiency": 0.0}
        print("%8.2f  %8.3f %8.3f %8.3f  %8.3f %8.3f %8.3f"
              % (frac,
                 nn_m.get("efficiency", 0.0), nn_m.get("fake_rate", 0.0),
                 nn_m.get("recurl_efficiency", 0.0),
                 bl["efficiency"], bl["fake_rate"], bl["recurl_efficiency"]))
    print("wrote %s" % (out_dir / "last.pt"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
