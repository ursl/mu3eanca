"""Same-``tid`` losses: object condensation (default) and supervised contrastive."""

from __future__ import annotations

from typing import Dict

import torch
import torch.nn.functional as F


def _frame_oc(
    emb: torch.Tensor,
    beta: torch.Tensor,
    tid: torch.Tensor,
    hit_mask: torch.Tensor,
    q_min: float,
) -> tuple[torch.Tensor, torch.Tensor, torch.Tensor, torch.Tensor]:
    """Object-condensation terms for one frame (padded 1-D)."""
    valid = hit_mask.bool()
    zero = emb.new_zeros(())
    if int(valid.sum()) == 0:
        return zero, zero, zero, zero

    x = emb[valid]
    b = beta[valid]
    t = tid[valid]
    q = torch.atanh(b) ** 2 + q_min

    objects = t[t > 0].unique()
    if objects.numel() == 0:
        noise = b[t == 0]
        l_noise = noise.mean() if noise.numel() else zero
        return zero, zero, zero, l_noise

    l_att = zero
    l_rep = zero
    l_beta = zero
    n_obj = 0
    for obj in objects:
        m = t == obj
        if int(m.sum()) == 0:
            continue
        idx = torch.where(m)[0]
        k = idx[int(torch.argmax(b[idx]))]
        xk = x[k]
        qk = q[k]
        dist = torch.linalg.vector_norm(x - xk, dim=-1)
        l_att = l_att + (qk * dist[m].square()).mean()
        others = ~m
        if bool(others.any()):
            hinge = torch.relu(1.0 - dist[others])
            l_rep = l_rep + (qk * hinge.square()).mean()
        l_beta = l_beta + (1.0 - b[k])
        n_obj += 1

    n = float(max(n_obj, 1))
    noise = b[t == 0]
    l_noise = noise.mean() if noise.numel() else zero
    return l_att / n, l_rep / n, l_beta / n, l_noise


def condensation_loss(
    emb: torch.Tensor,
    beta: torch.Tensor,
    tid: torch.Tensor,
    hit_mask: torch.Tensor,
    q_min: float = 1.0,
    w_att: float = 1.0,
    w_rep: float = 1.0,
    w_beta: float = 2.0,
    w_noise: float = 1.0,
) -> tuple[torch.Tensor, Dict[str, float]]:
    """Kieseler object condensation, averaged over the batch.

    ``tid == 0`` is noise. Padding is ``hit_mask == False`` and is ignored.
    """
    bsz = emb.shape[0]
    parts = []
    for i in range(bsz):
        parts.append(_frame_oc(emb[i], beta[i], tid[i], hit_mask[i], q_min))
    att = torch.stack([p[0] for p in parts]).mean()
    rep = torch.stack([p[1] for p in parts]).mean()
    lbeta = torch.stack([p[2] for p in parts]).mean()
    noise = torch.stack([p[3] for p in parts]).mean()
    total = w_att * att + w_rep * rep + w_beta * lbeta + w_noise * noise
    stats = {
        "loss": float(total.detach()),
        "att": float(att.detach()),
        "rep": float(rep.detach()),
        "beta": float(lbeta.detach()),
        "noise": float(noise.detach()),
    }
    return total, stats


def contrastive_loss(
    emb: torch.Tensor,
    tid: torch.Tensor,
    hit_mask: torch.Tensor,
    tau: float = 0.1,
) -> tuple[torch.Tensor, Dict[str, float]]:
    """Supervised contrastive pull of same-``tid`` hits, one frame at a time."""
    bsz = emb.shape[0]
    losses = []
    zero = emb.new_zeros(())
    for i in range(bsz):
        valid = hit_mask[i].bool()
        if int(valid.sum()) < 2:
            losses.append(zero)
            continue
        x = F.normalize(emb[i][valid], dim=-1)
        t = tid[i][valid]
        sim = x @ x.T / tau
        eye = torch.eye(x.shape[0], dtype=torch.bool, device=x.device)
        same = t[:, None] == t[None, :]
        pos = same & ~eye & (t[:, None] > 0)
        logits = sim.masked_fill(eye, float("-inf"))
        log_prob = logits - torch.logsumexp(logits, dim=-1, keepdim=True)
        npos = pos.sum(dim=-1)
        has = npos > 0
        if not bool(has.any()):
            losses.append(zero)
            continue
        pull = -(log_prob * pos).sum(dim=-1)[has] / npos[has].to(x.dtype)
        losses.append(pull.mean())
    total = torch.stack(losses).mean() if losses else zero
    return total, {"loss": float(total.detach()), "att": 0.0, "rep": 0.0, "beta": 0.0, "noise": 0.0}
