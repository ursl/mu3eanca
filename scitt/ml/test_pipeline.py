"""Smoke tests that do not need a ROOT dump."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

_SCITT = Path(__file__).resolve().parents[1]
if str(_SCITT) not in sys.path:
    sys.path.insert(0, str(_SCITT))

import numpy as np

from ml.dataset import collate_encoded, encode_frame, group_hits_by_frame, n_hits
from ml.features import FEATURE_DIM, LABEL_COLUMNS, encode_hits, assert_no_label_leak
from ml.kill import kill_hits
from ml.metrics import MetricAccum
from ml.synthetic import generate_frame, make_synthetic_dataset


class FeaturesTests(unittest.TestCase):
    def test_no_label_leak(self) -> None:
        assert_no_label_leak()
        for name in LABEL_COLUMNS:
            self.assertNotIn(name, ("x", "y", "z", "r", "phi", "layer", "tot", "time"))

    def test_encode_shape(self) -> None:
        rng = np.random.default_rng(0)
        fr = generate_frame(1, rng)
        feats = encode_hits(fr)
        self.assertEqual(feats.shape, (n_hits(fr), FEATURE_DIM))
        self.assertEqual(feats.dtype, np.float32)
        self.assertTrue(np.isfinite(feats).all())


class DatasetTests(unittest.TestCase):
    def test_group_by_frame(self) -> None:
        cols = {
            "frame_id": np.array([7, 7, 8, 7], dtype=np.uint64),
            "x": np.array([1.0, 2.0, 3.0, 4.0]),
            "y": np.zeros(4),
            "z": np.zeros(4),
            "r": np.ones(4),
            "phi": np.zeros(4),
            "layer": np.zeros(4, dtype=np.int64),
            "tot": np.ones(4, dtype=np.int64),
            "time": np.zeros(4),
            "tid": np.array([1, 1, 2, 0]),
            "pid": np.array([11, 11, 11, 0]),
            "hid": np.array([1, 2, 1, 0]),
            "sensor_id": np.arange(4),
            "n_mc": np.array([1, 1, 1, 0]),
        }
        frames = group_hits_by_frame(cols)
        self.assertEqual([fr["frame_id"] for fr in frames], [7, 8])
        self.assertEqual(n_hits(frames[0]), 3)
        self.assertEqual(n_hits(frames[1]), 1)

    def test_collate_padding(self) -> None:
        try:
            import torch  # noqa: F401
        except ImportError:
            self.skipTest("torch not installed")
        rng = np.random.default_rng(1)
        frames = [encode_frame(generate_frame(i, rng)) for i in range(3)]
        batch = collate_encoded(frames)
        self.assertEqual(batch["features"].shape[0], 3)
        self.assertEqual(batch["features"].shape[-1], FEATURE_DIM)
        self.assertTrue(batch["hit_mask"].any())
        for i, fr in enumerate(frames):
            n = fr["features"].shape[0]
            self.assertTrue(bool(batch["hit_mask"][i, :n].all()))
            if n < batch["hit_mask"].shape[1]:
                self.assertFalse(bool(batch["hit_mask"][i, n:].any()))


class KillTests(unittest.TestCase):
    def test_sensor_kill_reduces_hits(self) -> None:
        rng = np.random.default_rng(2)
        fr = generate_frame(0, rng)
        killed = kill_hits(fr, rng, mode="sensor", fraction=1.0)
        # fraction=1 drops every sensor; implementation keeps the original
        # if that would empty the frame.
        self.assertGreater(n_hits(killed), 0)
        killed = kill_hits(fr, np.random.default_rng(3), mode="hit", fraction=0.5)
        self.assertLessEqual(n_hits(killed), n_hits(fr))
        self.assertGreater(n_hits(killed), 0)

    def test_layer_kill(self) -> None:
        rng = np.random.default_rng(4)
        fr = generate_frame(0, rng)
        killed = kill_hits(fr, rng, mode="layer", fraction=0.0)
        self.assertEqual(n_hits(killed), n_hits(fr))


class MetricsTests(unittest.TestCase):
    def test_perfect_clustering(self) -> None:
        tid = np.array([1, 1, 1, 2, 2, 0, 0])
        pred = np.array([0, 0, 0, 1, 1, -1, -1])
        acc = MetricAccum()
        acc.add(tid, pred, min_hits=2)
        m = acc.as_dict()
        self.assertAlmostEqual(m["efficiency"], 1.0)
        self.assertAlmostEqual(m["fake_rate"], 0.0)

    def test_recurl_counts(self) -> None:
        tid = np.array([1, 1, 1, 1])
        pred = np.array([0, 0, 0, 0])
        hid = np.array([1, 2, 3, -4])
        acc = MetricAccum()
        acc.add(tid, pred, hid=hid, min_hits=2)
        self.assertEqual(acc.n_recurl_truth, 1)
        self.assertEqual(acc.n_recurl_found, 1)


class ModelTests(unittest.TestCase):
    def test_forward_and_step(self) -> None:
        try:
            import torch
        except ImportError:
            self.skipTest("torch not installed")
        from ml.loss import condensation_loss
        from ml.model import HitTransformer
        from ml.train import train_one_epoch

        ds = make_synthetic_dataset(8, seed=5)
        from ml.kill import make_kill_collate
        from torch.utils.data import DataLoader

        loader = DataLoader(
            ds, batch_size=2, collate_fn=make_kill_collate("sensor", 0.2, True, seed=5),
        )
        device = torch.device("cpu")
        model = HitTransformer(d_model=32, nhead=4, nlayers=2, dim_ff=64, emb_dim=4).to(device)
        batch = next(iter(loader))
        emb, beta = model(batch["features"], batch["hit_mask"])
        self.assertEqual(emb.shape[:2], batch["features"].shape[:2])
        self.assertEqual(beta.shape, batch["hit_mask"].shape)
        loss, _ = condensation_loss(emb, beta, batch["tid"], batch["hit_mask"])
        self.assertTrue(torch.isfinite(loss))
        opt = torch.optim.Adam(model.parameters(), lr=1e-3)
        stats = train_one_epoch(model, loader, opt, device, "oc")
        self.assertIn("loss", stats)

    def test_eval_drop_scan_runs(self) -> None:
        try:
            import torch
        except ImportError:
            self.skipTest("torch not installed")
        from ml.model import HitTransformer
        from ml.train import evaluate

        ds = make_synthetic_dataset(4, seed=6)
        device = torch.device("cpu")
        model = HitTransformer(d_model=32, nhead=4, nlayers=1, dim_ff=64, emb_dim=4).to(device)
        m0 = evaluate(ds, model=model, device=device, kill_mode="none", min_hits=2)
        m1 = evaluate(ds, model=model, device=device, kill_mode="sensor", kill_frac=0.3, min_hits=2)
        b0 = evaluate(ds, use_baseline=True, kill_mode="none", min_hits=2)
        self.assertIn("efficiency", m0)
        self.assertIn("efficiency", m1)
        self.assertIn("efficiency", b0)


if __name__ == "__main__":
    unittest.main()
