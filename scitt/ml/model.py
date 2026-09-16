"""Hit-set encoder: embed → TransformerEncoder → per-hit embedding (+ β)."""

from __future__ import annotations

import torch
import torch.nn as nn
from ml.features import FEATURE_DIM


class HitTransformer(nn.Module):
    """Global self-attention over the hits of one frame.

    Input is reconstructed features only. Output:
      ``emb``  clustering coordinates ``[B, N, emb_dim]``
      ``beta`` condensation score in (0, 1) ``[B, N]``
    """

    def __init__(
        self,
        in_dim: int = FEATURE_DIM,
        d_model: int = 64,
        nhead: int = 4,
        nlayers: int = 2,
        dim_ff: int = 128,
        emb_dim: int = 8,
        dropout: float = 0.1,
    ) -> None:
        super().__init__()
        if d_model % nhead != 0:
            raise ValueError("d_model must be divisible by nhead")
        self.input = nn.Sequential(
            nn.Linear(in_dim, d_model),
            nn.GELU(),
            nn.LayerNorm(d_model),
        )
        enc_layer = nn.TransformerEncoderLayer(
            d_model=d_model,
            nhead=nhead,
            dim_feedforward=dim_ff,
            dropout=dropout,
            activation="gelu",
            batch_first=True,
            norm_first=True,
        )
        try:
            self.encoder = nn.TransformerEncoder(
                enc_layer, num_layers=nlayers, enable_nested_tensor=False
            )
        except TypeError:
            self.encoder = nn.TransformerEncoder(enc_layer, num_layers=nlayers)
        self.to_emb = nn.Linear(d_model, emb_dim)
        self.to_beta = nn.Linear(d_model, 1)

    def forward(
        self,
        features: torch.Tensor,
        hit_mask: torch.Tensor,
    ) -> tuple[torch.Tensor, torch.Tensor]:
        h = self.input(features)
        pad = ~hit_mask
        h = self.encoder(h, src_key_padding_mask=pad)
        emb = self.to_emb(h)
        beta = torch.sigmoid(self.to_beta(h).squeeze(-1)).clamp(1e-4, 1.0 - 1e-4)
        mask_f = hit_mask.to(dtype=emb.dtype).unsqueeze(-1)
        emb = emb * mask_f
        beta = beta * hit_mask.to(dtype=beta.dtype)
        return emb, beta


def pick_device(name: str = "auto") -> torch.device:
    if name == "auto":
        if torch.cuda.is_available():
            return torch.device("cuda")
        if getattr(torch.backends, "mps", None) and torch.backends.mps.is_available():
            return torch.device("mps")
        return torch.device("cpu")
    return torch.device(name)
