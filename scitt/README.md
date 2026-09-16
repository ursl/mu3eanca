# scitt

Silicon-hit dump and frame-level transformer track finding for Mu3e (3-layer).

One training example is **one frame**. Tokens are reconstructed hits.
`tid` / `hid` / `pid` / truth position are **labels only** — never model input.
The physics question is whether a global association still finds a track when
hits are missing (dead sensors / chips), instead of failing when the
extrapolated “next” layer is dead.

Commands below are **tcsh**. Python and docs are run from **this directory**.
Dumper build/run uses the reconstruction tree. Paths match this machine;
adjust if yours differ.

| | |
|---|---|
| this directory | `/Users/ursl/macros/ana/mu3eanca/scitt` |
| analysis macros | `/Users/ursl/macros/ana/mu3eanca` |
| reconstruction | `/Users/ursl/mu3e/software/v7.1-analysis` |
| dumper binary | `/Users/ursl/mu3e/software/v7.1-analysis/_build/mu3eTrirec/runScittDumper` |

The ML plan (not run instructions) is in [`TRANSFORMER_HANDOFF.md`](TRANSFORMER_HANDOFF.md).

## Layout

```
scitt/
  scittHit.hh scittTrack.hh scittTree.hh scittTree.cc   # ROOT dump helpers
  ml/          PyTorch dataset, encoder, train, eval
  .venv/       local Python env (gitignored)
  runs/        checkpoints (gitignored)
```

---

## 1. Dump hits to ROOT

The executable is `runScittDumper` (`mu3eTrirec/runScittDumper.cpp` plus the
helpers in this directory). CMake finds this folder with `-DSCITT_DIR=...`
or, if set, `$MU3EANCA/scitt`.

### Build / rebuild

```tcsh
setenv SCITT /Users/ursl/macros/ana/mu3eanca/scitt
cd /Users/ursl/mu3e/software/v7.1-analysis/_build
cmake -DSCITT_DIR=$SCITT /Users/ursl/mu3e/software/v7.1-analysis
make -j`sysctl -n hw.ncpu` runScittDumper
```

If `SCITT_DIR` was cached from the old `scittDumper` name, the `cmake` line
above resets it.

### Run

Same CLI as other Mu3e rec tools: positional sim ROOT input, optional `--output`
for a trirec file, `-n` / `-s` for frame count / skip, `-v` for verbosity.

```tcsh
set DUMP = /Users/ursl/mu3e/software/v7.1-analysis/_build/mu3eTrirec/runScittDumper

# 100 frames; scitt tree written next to --output (see naming below)
$DUMP -n 100 -v --output /tmp/trirec_out.root /path/to/sim.root

# stdout only for trirec; still writes a scitt ROOT file
$DUMP -n 20 -v /path/to/sim.root
```

| flag | |
|---|---|
| `-n N` | process N frames |
| `-s N` / `--skip N` | skip N frames first |
| `--output FILE` | trirec ROOT (optional). Also used to name the scitt file |
| `-v` | per-frame hit/track counts |
| `-v -v` | full verbose dump grouped by truth `tid` |

**Scitt output name** (printed as `scittOutput:`):

| `--output` | scitt file |
|---|---|
| omitted | `scitt.root` in the current directory |
| `.../foo_run....root` | `.../scitt_run....root` (stem before `_run` replaced) |
| any other `....root` | `...._scitt.root` |

That file has two **flat** trees, one row per object. Group in Python by `frame_id`.

### Trees

**`hits`** — one reconstructed pixel.

Model input: `x,y,z,r,phi,layer,tot,time` (optional extras: `sensor_id,row,col,ts`).

Labels / debug, **not input**:

| branch | |
|---|---|
| `tid` | Geant4 trajectory id. **`tid == 0` is unmatched / noise** |
| `pid` | PDG (`11` e⁻, `-11` e⁺). `0` means unset, not a particle type |
| `hid` / `abs_hid` | signed silicon-crossing index; do not use as input |
| `n_mc` | how many truth particles contributed; v1 uses `mcs.front()` only |
| `mc_x,y,z` | Geant4 truth position |

**`tracks`** — truth particles with ≥1 labelled Si hit (`tid`, `pid`, `n_hits`,
`px,py,pz`, `vx,vy,vz`). Filter targets; not transformer input.

`layer` is 0-based (L1, L2, L3). Recurls through L3 are normal (`hid` sign flips
inward). Shared pixels (`n_mc > 1`) keep the primary `tid` only.

---

## 2. Python environment

All Python lives under this directory. `cd` here first:

```tcsh
cd /Users/ursl/macros/ana/mu3eanca/scitt

# first time (already created on this machine)
python3 -m venv .venv
source .venv/bin/activate.csh
python -m pip install torch numpy scikit-learn uproot

# later sessions
cd /Users/ursl/macros/ana/mu3eanca/scitt
source .venv/bin/activate.csh
```

`uproot` is only required to read a real dump (`--root`). `--synthetic` does
not use ROOT: it **invents** toy hits in Python (fake helices on the three
layer radii, optional L3 recurls, plus noise). That is only to exercise the
pipeline before a dump exists; it is not Geant4. Device is `auto`: CUDA, else
MPS (Apple), else CPU.

Deps are listed in [`pyproject.toml`](pyproject.toml).

---

## 3. Train and evaluate

From this directory, venv active. `--synthetic` and `--root` are mutually
exclusive; if you pass neither, training falls back to synthetic.

### Smoke (no dump needed)

Tiny model, few frames, a few epochs:

```tcsh
python -m ml.train --synthetic --fast --kill-mode sensor --kill-frac 0.3
```

### Proper synthetic run

Toy 3-layer frames with optional L3 recurls. Good for checking the pipeline
before a dump exists.

```tcsh
python -m ml.train --synthetic --epochs 20 --n-train 256 --n-val 64 \
  --kill-mode sensor --kill-frac 0.3 \
  --eval-fracs 0,0.15,0.3,0.5 \
  --out runs/synth
```

On this toy set, [DBSCAN](https://en.wikipedia.org/wiki/DBSCAN) on (φ, z, t) is
a strong baseline: it is ordinary density clustering (hits near each other
become a “track”), and the toy generator plants compact blobs. The transformer
is meant to be judged on **real dumps with killed sensors**.

### Real dump

```tcsh
python -m ml.train --root /path/to/scitt.root --epochs 20 --kill-mode sensor --kill-frac 0.3  --eval-fracs 0,0.15,0.3,0.5   --out runs/data
```

Cap frames or change the train/val split:

```tcsh
python -m ml.train --root /path/to/scitt.root --max-frames 500 --val-frac 0.2
```

Checkpoints: `runs/<name>/best.pt` (best val efficiency) and `last.pt`.
Default `--out` is `runs/` (next to `ml/`).

### Apply a trained model

The file `best.pt` / `last.pt` is the model (weights + architecture flags).
Applying it means: encode one frame of reconstructed hits → transformer →
cluster in embedding space. **Do not pass `tid`.** Output is one integer per
hit: `0,1,2,…` = cluster (a predicted track), `-1` = unassigned / noise.

Required per hit: `x, y, z, layer, tot, time`. Optional: `r`, `phi` (else
computed from `x,y`). Units as in the dump (mm, ns, 0-based layer).

```tcsh
python -m ml.infer --ckpt runs/best.pt --root /path/to/scitt.root
python -m ml.infer --ckpt runs/best.pt --root /path/to/scitt.root --frame-id 42
```

From Python (same directory / venv):

```python
from ml.infer import load_model, cluster_hits
import numpy as np

model, device, _ = load_model("runs/best.pt")
hits = {
    "x": np.array([...], dtype=np.float32),
    "y": np.array([...], dtype=np.float32),
    "z": np.array([...], dtype=np.float32),
    "layer": np.array([...], dtype=np.int64),   # 0,1,2
    "tot": np.array([...], dtype=np.float32),
    "time": np.array([...], dtype=np.float32),
}
labels = cluster_hits(model, hits, device)   # shape [N]
# hits with the same label belong to the same predicted track
```

There is no helix fit yet: this only assigns hits to clusters.

### What training does

1. Load frames (`hits` tree grouped by `frame_id`, or synthetic).
2. During training, randomly drop hits up to `--kill-frac` (`--kill-mode`).
3. 2-layer transformer encoder → per-hit embedding + condensation score β.
4. Loss: object condensation on `tid` (or `--loss contrastive`).
5. Inference: cluster embeddings (**no `tid`**). Recurls should stay one instance.
6. After the last epoch, print a **drop scan**: NN vs DBSCAN efficiency / fake
   rate / recurl efficiency vs drop fraction.

A track is reconstructable if it still has `--min-hits` hits after the drop
(default 2, so L1+L3 with a dead L2 still counts). Match: ≥50% efficiency and
≥50% purity vs a truth `tid`.

### Useful flags

| flag | default | |
|---|---|---|
| `--root` / `--synthetic` | synthetic if omitted | data source |
| `--epochs` | 10 | |
| `--batch-size` | 4 | frames per batch |
| `--n-train` `--n-val` | 128 / 32 | synthetic sizes |
| `--max-frames` | 0 (all) | ROOT cap |
| `--val-frac` | 0.2 | ROOT split |
| `--kill-mode` | `sensor` | `none` `hit` `sensor` `chip` `layer` (`chip` = `sensor`) |
| `--kill-frac` | 0.25 | max drop probability in training; eval scan uses `--eval-fracs` |
| `--eval-fracs` | `0,0.15,0.3,0.5` | drop scan after training |
| `--min-hits` | 2 | reconstructable after drop |
| `--loss` | `oc` | `oc` or `contrastive` |
| `--d-model` `--nlayers` `--nhead` | 64 / 2 / 4 | encoder size |
| `--device` | `auto` | `cpu` `mps` `cuda` |
| `--out` | `runs/` | checkpoint directory |
| `--no-baseline` | off | skip DBSCAN in the drop scan |
| `--fast` | off | shrink model / frames / epochs |
| `--seed` | 0 | |

```tcsh
python -m ml.train -h
```

---

## 4. Tests

No ROOT file required:

```tcsh
cd /Users/ursl/macros/ana/mu3eanca/scitt
source .venv/bin/activate.csh
python -m unittest ml.test_pipeline
```

---

## 5. Model input vs labels

Encoded hit vector (11 numbers): `x,y,z,r` (scaled), `sin φ`, `cos φ`,
layer one-hot (L1–L3), `tot`, time relative to the frame median / 64 ns.

`tid`, `hid`, `pid`, `n_mc`, `mc_*` are **not** in that vector. The loader
asserts this (`assert_no_label_leak`).
