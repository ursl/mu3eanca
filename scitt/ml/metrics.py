"""Tracking efficiency / fake rate from predicted clusters vs truth ``tid``."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import numpy as np


@dataclass
class MetricAccum:
    n_truth: int = 0
    n_found: int = 0
    n_pred: int = 0
    n_fake: int = 0
    n_hit_ok: int = 0
    n_hit: int = 0
    n_recurl_truth: int = 0
    n_recurl_found: int = 0
    kill_sum: float = 0.0
    n_frames: int = 0

    def add(
        self,
        tid: np.ndarray,
        pred: np.ndarray,
        hid: Optional[np.ndarray] = None,
        min_hits: int = 2,
        min_purity: float = 0.5,
        min_eff: float = 0.5,
        kill_frac: float = 0.0,
    ) -> None:
        tid = np.asarray(tid, dtype=np.int64)
        pred = np.asarray(pred, dtype=np.int64)
        self.n_frames += 1
        self.kill_sum += kill_frac

        truth_ids = [int(t) for t in np.unique(tid) if t > 0]
        pred_ids = [int(p) for p in np.unique(pred) if p >= 0]

        matched_pred: set[int] = set()
        for t in truth_ids:
            m = tid == t
            nh = int(m.sum())
            if nh < min_hits:
                continue
            self.n_truth += 1
            is_recurl = False
            if hid is not None:
                h = np.asarray(hid)[m]
                is_recurl = bool(np.any(h > 0) and np.any(h < 0))
                if is_recurl:
                    self.n_recurl_truth += 1

            best_p = None
            best_ov = -1
            for p in pred_ids:
                ov = int(np.sum(m & (pred == p)))
                if ov > best_ov:
                    best_ov = ov
                    best_p = p
            if best_p is None or best_ov <= 0:
                continue
            n_cl = int(np.sum(pred == best_p))
            eff = best_ov / float(nh)
            pur = best_ov / float(max(n_cl, 1))
            if eff >= min_eff and pur >= min_purity:
                self.n_found += 1
                matched_pred.add(best_p)
                if is_recurl:
                    self.n_recurl_found += 1

        for p in pred_ids:
            n_cl = int(np.sum(pred == p))
            if n_cl < min_hits:
                continue
            self.n_pred += 1
            if p not in matched_pred:
                self.n_fake += 1

        labelled = tid > 0
        self.n_hit += int(labelled.sum())
        if labelled.any() and pred_ids:
            for t in np.unique(tid[labelled]):
                m = tid == t
                if not np.any(m):
                    continue
                # majority predicted label among this particle's hits
                labs, cnts = np.unique(pred[m], return_counts=True)
                maj = labs[int(np.argmax(cnts))]
                if maj >= 0:
                    self.n_hit_ok += int(np.sum(m & (pred == maj)))

    def as_dict(self) -> dict[str, float]:
        def _r(num: int, den: int) -> float:
            return float(num) / float(den) if den else 0.0

        return {
            "efficiency": _r(self.n_found, self.n_truth),
            "fake_rate": _r(self.n_fake, self.n_pred),
            "hit_purity": _r(self.n_hit_ok, self.n_hit),
            "recurl_efficiency": _r(self.n_recurl_found, self.n_recurl_truth),
            "n_truth": float(self.n_truth),
            "n_found": float(self.n_found),
            "n_pred": float(self.n_pred),
            "n_fake": float(self.n_fake),
            "mean_kill": self.kill_sum / float(self.n_frames) if self.n_frames else 0.0,
            "n_frames": float(self.n_frames),
        }


def format_metrics(name: str, m: dict[str, float]) -> str:
    return (
        "%s  eff=%.3f  fake=%.3f  hit=%.3f  recurl=%.3f  "
        "kill=%.3f  n_truth=%d  n_frames=%d"
        % (
            name,
            m["efficiency"],
            m["fake_rate"],
            m["hit_purity"],
            m["recurl_efficiency"],
            m["mean_kill"],
            int(m["n_truth"]),
            int(m["n_frames"]),
        )
    )
