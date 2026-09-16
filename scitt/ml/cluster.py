"""Inference clustering. No ``tid`` is used."""

from __future__ import annotations

import numpy as np
import torch


def cluster_condensation(
    emb: np.ndarray | torch.Tensor,
    beta: np.ndarray | torch.Tensor,
    hit_mask: np.ndarray | torch.Tensor | None = None,
    t_beta: float = 0.2,
    t_dist: float = 0.8,
) -> np.ndarray:
    """Greedy object-condensation clustering.

    Highest-β hits with β > ``t_beta`` seed a cluster; unassigned hits
    within Euclidean ``t_dist`` of a seed join it. Remainder is noise (−1).
    """
    x = _np(emb)
    b = _np(beta).reshape(-1)
    n = x.shape[0]
    if hit_mask is None:
        valid = np.ones(n, dtype=bool)
    else:
        valid = _np(hit_mask).astype(bool).reshape(-1)
    labels = np.full(n, -1, dtype=np.int64)
    if not np.any(valid):
        return labels

    order = np.argsort(-b)
    next_id = 0
    assigned = np.zeros(n, dtype=bool)
    assigned[~valid] = True
    for i in order:
        if assigned[i] or not valid[i] or b[i] < t_beta:
            continue
        dist = np.linalg.norm(x - x[i], axis=-1)
        members = valid & ~assigned & (dist <= t_dist)
        members[i] = True
        labels[members] = next_id
        assigned[members] = True
        next_id += 1

    # Early training often has no β above threshold; still cluster embeddings.
    if next_id == 0:
        return cluster_dbscan_embeddings(x, valid, eps=max(t_dist, 0.4), min_samples=2)
    return labels


def cluster_dbscan_embeddings(
    emb: np.ndarray | torch.Tensor,
    hit_mask: np.ndarray | torch.Tensor | None = None,
    eps: float = 0.6,
    min_samples: int = 2,
) -> np.ndarray:
    from sklearn.cluster import DBSCAN

    x = _np(emb)
    n = x.shape[0]
    labels = np.full(n, -1, dtype=np.int64)
    if hit_mask is None:
        valid = np.ones(n, dtype=bool)
    else:
        valid = _np(hit_mask).astype(bool).reshape(-1)
    if int(valid.sum()) < min_samples:
        return labels
    pred = DBSCAN(eps=eps, min_samples=min_samples).fit_predict(x[valid])
    labels[valid] = pred.astype(np.int64)
    return labels


def _np(t) -> np.ndarray:
    if isinstance(t, torch.Tensor):
        return t.detach().cpu().numpy()
    return np.asarray(t)
