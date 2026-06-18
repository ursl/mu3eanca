# MU3E ReReco (Snakemake)

Offline reprocessing workflows on **existing data** with a configurable MU3E tag and/or CDB global tag. Unlike `prompt/relval/`, there is no simulation or sort step — each **task** runs a chosen **action** on inputs you point at.

Actions are pluggable; today:

| `action` | Tool | Input |
|----------|------|--------|
| `trirec` | `mu3eTrirec` | Existing sort ROOT file |
| `midas_meta` | `mu3eUtil/.../mu3e_midas_meta` | Directory of raw `*.mid.lz4` files |

## Workflow overview

1. **`initRereco`** — once per setup: write `setups/<name>/config.yaml`, copy Snakefile + `common/`.
2. **`snakemake mu3e_setup`** — always first: clone/checkout MU3E (+ submodules), build, relink.
3. **`snakemake midas_meta_all`** / **`trirec_all`** — operational runs (run list, batch/local via `--config`).

Setup parameters (`mu3e_tag`, branch, GT, paths, templates) live in `setups/<name>/config.yaml`. Operational parameters (`min_run`, `max_run`, `run_list_file`, `job_submit_mode`) are passed per snakemake invocation via `--config`.

## What the workflow does

**Shared infrastructure** (`mu3e_setup`):

| Step | Description |
|------|-------------|
| Clone & prepare mu3e | Checkout tag/branch (+ optional merges in mu3e and mu3eUtil) |
| Build mu3e | `cmake ..` + `make` in `mu3e/_build` |
| Relink | `relinkBinFiles`; ensure `run/bvr2026` exists |

**Per task** (from `rereco_task_templates` + run list):

- **`trirec`** — `mu3eTrirec` on a sort file → `run/output/trirec-{task}.root`
- **`midas_meta`** — `mu3e_midas_meta` on each `*.mid.lz4` → marker `.markers/midas_meta-{task}.done`

| Target | Meaning |
|--------|---------|
| `mu3e_setup` | Clone/checkout, build, relink — **run this first** |
| `midas_meta_all` | All midas_meta markers for configured runs |
| `trirec_all` | All trirec outputs for configured runs |

## Layout on disk

```
$mu3e_rereco_basedir/
  setups/260618-midasMeta/          # Snakemake cwd: Snakefile, config.yaml, common/
  mu3e-260618-midasMeta/            # workdir (SETUP_ROOT): mu3e/, .markers/, logs/
    mu3e/
      _build/modules/mu3eUtil/tools/midasMeta/mu3e_midas_meta
      run/output/trirec-{task}.root
    logs/slurm/                     # batch midas_meta SLURM logs (default)
```

Host paths: `config-<hostname>.yaml` (e.g. `config-merlin-l-001.yaml`). Override: `REREC_HOST=moor`.

## Running

### 1. Create setup (once)

```tcsh
cd /path/to/mu3eanca/prompt/rereco

./initRereco -s 260618-midasMeta -t dev -b dev -g datav6.5=2025V1

# optional setup overrides:
./initRereco -s 260618-midasMeta -t dev -b dev -g datav6.5=2025V1 \
  --config mu3e_checkout_merge=75d8c48 mu3eUtil_checkout_merge=a1b2c3d4
```

Re-run `initRereco` to refresh `Snakefile`/`common/` and update setup config (same `-s -t -b -g`).

### 2. MU3E setup (always first)

```tcsh
set WF = /data/experiment/mu3e/data/prod/rereco/setups/260618-midasMeta
cd $WF

snakemake --cores 4 -p mu3e_setup
```

Dry-run: add `-n`.

### 3. Operational: midas_meta (local)

```tcsh
cd $WF

snakemake --cores 4 -p \
  --config min_run=4756 max_run=4799 \
  run_list_file=/psi/home/langenegger/mu3e/mu3eanca/db0/cdb2/certification/2025/2025-Beam-v1-significant.run \
  -- midas_meta_all
```

### 4. Operational: midas_meta (batch / SLURM)

```tcsh
snakemake --cores 10 -p \
  --config min_run=4756 max_run=4799 job_submit_mode=batch \
  run_list_file=/psi/home/langenegger/mu3e/mu3eanca/db0/cdb2/certification/2025/2025-Beam-v1-significant.run \
  -- midas_meta_all
```

`--cores N` = up to N concurrent jobs (local perl or `sbatch --wait`). SLURM logs: `<workdir>/logs/slurm/`.

### Refresh workflow code only

```tcsh
./initRereco -s 260618-midasMeta -t dev -b dev -g datav6.5=2025V1
cd $WF && snakemake ...
```

## Configuration

See `config.yaml` and `config-<hostname>.yaml`.

### Run lists

```yaml
run_list_file: "/abs/path/to/certification.runs"
min_run: 4756
max_run: 4799

rereco_task_templates:
  - action: midas_meta
    input_file_tpl: "run{run:05d}.mid.lz4"
```

Or pass bounds on the CLI: `--config min_run=4756 max_run=4799 run_list_file=/abs/path -- midas_meta_all`.

Template placeholders: `{run}`, `{run:05d}`, `{runblock:03d}`, `{runblock:04d}`. Set `raw_input_layout: runblock3` for merlin raw paths (`raw/004/run04756.mid.lz4`).

### MU3E checkout merges

**mu3e** (main repo), after checkout:

```yaml
mu3e_checkout_merge: "75d8c48"
```

**mu3eUtil** (submodule at `modules/mu3eUtil`), after `git submodule update`:

```yaml
mu3eUtil_checkout_merge: "a1b2c3d4"
```

Repeatable lists: `mu3e_checkout_merges` / `mu3eUtil_checkout_merges`.

### midas_meta batch mode

In setup config or per run:

```yaml
job_submit_mode: "batch"    # or "local" (default)
slurm_partition: "mu3e"
slurm_batch_script: "/path/to/mu3eanca/slurm/slurm-midas_meta.csh"
```

Override per run: `--config job_submit_mode=batch` (use `--` before the target if it follows other `--config` values).

When passing several `--config key=value` entries, put the snakemake target after `--` so it is not parsed as config:

```tcsh
snakemake --cores 4 -p --config min_run=4756 max_run=4799 -- midas_meta_all
```

## Shared code with relval

MU3E checkout/build/relink live under `prompt/common/`. `initRereco` copies `common/` into each setup directory.

## Logs

- `logs/snakemake/run_midas_meta-{task}.log`
- `<workdir>/logs/slurm/midas-{run}.{jobid}.out` (batch mode)
