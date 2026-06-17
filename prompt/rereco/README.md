# MU3E ReReco (Snakemake)

Offline reprocessing workflows on **existing data** with a configurable MU3E tag and/or CDB global tag. Unlike `prompt/relval/`, there is no simulation or sort step — each **task** runs a chosen **action** on inputs you point at.

Actions are pluggable; today:

| `action` | Tool | Input |
|----------|------|--------|
| `trirec` | `mu3eTrirec` | Existing sort ROOT file |
| `midas_meta` | `mu3eUtil/.../mu3e_midas_meta` | Directory of raw `*.mid.lz4` files |

More actions can be added later (same task list, new `action` values and rules).

## What the workflow does

**Shared infrastructure** (only what your tasks need):

| Step | Used by | Description |
|------|---------|-------------|
| Clone & prepare mu3e | all actions | Checkout `mu3e_tag` / branch (+ submodules) |
| Build mu3e | all actions | `cmake ..` + `make` in `mu3e/_build` (tools under `_build/modules/*`) |
| Relink | `trirec` | `relinkBinFiles`; ensure `run/bvr2026` exists |

**Per task** (from `rereco_tasks` in `config.yaml`):

- **`trirec`** — `mu3eTrirec` on a sort file → `run/output/trirec-{task}.root`
- **`midas_meta`** — `mu3e_midas_meta` on every `*.mid.lz4` in `input_dir` → marker `.markers/midas_meta-{task}.done` (ROOT meta files are written by the tool next to the MID inputs)

Default target (`rule all`): all outputs/markers for every configured task. With no tasks configured, `all` is equivalent to **`mu3e_setup`**.

| Target | Meaning |
|--------|---------|
| `mu3e_setup` | Clone/checkout MU3E, `cmake`+`make`, relink — no `midas_meta` or `trirec` |
| `midas_meta_all` | All midas_meta markers for configured runs |
| `trirec_all` | All trirec outputs for configured runs |

## Layout on disk

| Setting | Meaning |
|--------|---------|
| `mu3e_rereco_basedir` | Root for setup workdirs and `setups/` (Snakemake metadata) |
| `mu3e_dir` | Name of the MU3E checkout subdirectory inside each setup (default: `mu3e`) |
| `sort_input_base` | Optional prefix for relative `sort_input` / `input_dir` / `input_file` paths |
| `raw_input_layout` | `flat` (default), `runblock3`, or `runblock4` — inserts a block subdir for relative midas paths |
| `run_block_digits` | Width for bare `{runblock}` in templates (default `3`; use `4` for CDB-style paths) |

After `./runRereco --setup-name rereco-v67 ...`:

```
$mu3e_rereco_basedir/
  setups/rereco-v67/                         # copied Snakefile + common/ (not the workdir)
  mu3e-rereco-v67/                           # Snakemake workdir (SETUP_ROOT)
    .markers/                                # workflow markers
    logs/snakemake/
    mu3e/                                    # MU3E git checkout + submodules
      _build/modules/mu3eUtil/tools/midasMeta/mu3e_midas_meta
      run/output/trirec-{task}.root
```

## Configuration

Edit `config.yaml` and `config-<hostname>.yaml`.

### Run lists from certification files

Run numbers live in comma-separated ASCII files under `db0/cdb2/certification/` (extension `.runs` or `.run`). Instead of listing every run in `rereco_tasks`, set **`run_list_file`** and **`rereco_task_templates`**. The Snakefile expands one task per run × template.

Filter the list with **`min_run`** / **`max_run`**: only runs with `min_run <= run <= max_run` are kept (`0` = no bound on that side).

```yaml
run_list_file: "../../db0/cdb2/certification/2025/2025-significant-V1.runs"
min_run: 5700
max_run: 5800

rereco_task_templates:
  - action: midas_meta
    input_file_tpl: "/data/raw/run{run:05d}.mid.lz4"
  - action: trirec
    sort_input_tpl: "/data/sort/run{run}/sort.root"
    trirec_conf: "trirec_twolayer_beam.conf"
    trirec_conf_fallback: "trirec.conf"
```

Template placeholders: `{run}` (plain number), `{run:05d}` (zero-padded), `{runblock:03d}` / `{runblock:04d}` (run `// 1000`, zero-padded block index). Bare `{runblock}` uses `run_block_digits` (default 3). Optional `id_tpl` defaults to `run{run}-{action}`.

**Run-block directories:** block index is always `run // 1000`. Non-CDB raw MID files use **3-digit** dirs (`000` = runs 0–999, `004` = 4000–4999). CDB payload trees under `db0/cdb2` use **4-digit** dirs (`0000`, `0004`, … — same block rule). Either put `{runblock:03d}/` in the template, or set `raw_input_layout: runblock3` with a relative `input_file_tpl` and `sort_input_base`.

When `run_list_file` is set, or when **`min_run`** is set with **`rereco_task_templates`**, explicit `rereco_tasks` are ignored.

**Single run** (no certification file; uses default `input_file_tpl` + `sort_input_base`):

```tcsh
./runRereco -s rereco-v67 -m 4756 -M 4756 -t dev -b dev -g datav6.5=2025V1 \
  --config sort_input_base=/Users/ursl/data/mu3e/run2025 midas_meta_all
```

**Batch from certification file** (written into setup `config.yaml`):

```tcsh
./runRereco -n -p -m 5700 -M 5800 -t v6.7 -g datav6.5=2025V1
```

Same bounds via Snakemake config override:

```tcsh
./runRereco -n -p --config min_run=5700 max_run=5800 -t v6.7 -g datav6.5=2025V1
```

### `trirec` task

```yaml
rereco_tasks:
  - id: "run5700-trirec"
    action: trirec
    run_id: 5700
    sort_input: "/data/mu3e/sort-run5700.root"
    trirec_conf: "trirec_twolayer_beam.conf"
    trirec_conf_fallback: "trirec.conf"
```

`sort_input` may be absolute or relative to `sort_input_base`. Conf files are resolved under `$MU3E_DIR/run/` (same as relval).

### `midas_meta` task

Per-run (typical with run-list expansion):

**Flat layout** (files directly under one directory, e.g. `run2025/`):

```yaml
sort_input_base: "/Users/ursl/data/mu3e/run2025"
raw_input_layout: flat
rereco_task_templates:
  - action: midas_meta
    input_file_tpl: "run{run:05d}.mid.lz4"
```

**3-digit run-block layout** (non-CDB raw, e.g. `raw/004/run04756.mid.lz4`):

```yaml
sort_input_base: "/Users/ursl/mu3e/raw"
raw_input_layout: runblock3
rereco_task_templates:
  - action: midas_meta
    input_file_tpl: "run{run:05d}.mid.lz4"
```

Equivalent explicit template (no `raw_input_layout` needed):

```yaml
  - action: midas_meta
    input_file_tpl: "{runblock:03d}/run{run:05d}.mid.lz4"
```

For CDB payload paths (4-digit blocks), use `{runblock:04d}` or `raw_input_layout: runblock4`.

Or a whole directory (all `*.mid.lz4` in that dir):

```yaml
  - id: "run5700-midas"
    action: midas_meta
    input_dir: "/data/raw/run5700"
```

Single-file mode runs one `mu3e_midas_meta` invocation; directory mode processes every `*.mid.lz4` (same pattern as `run2025/scripts/slurm-midasMeta.csh`). Requires full MU3E checkout and build (`mu3e/_build/`); the executable defaults to `_build/modules/mu3eUtil/tools/midasMeta/mu3e_midas_meta` (override with `midas_meta_exe` in config).

Optional per-task override: `input_file_base` / `input_dir_base` if paths are relative to something other than `sort_input_base`.

### Legacy name

`rereco_jobs` is still accepted; entries without `action` default to `trirec`.

### MU3E checkout: release tag vs branch HEAD

Default: checkout `mu3e_checkout_tag` (falls back to `mu3e_tag`), e.g. `v6.7`.

For the current tip of `dev`:

```yaml
mu3e_tag: "dev"
mu3e_checkout_branch: "dev"
```

```tcsh
./runRereco -n -p -t dev -b dev -g datav6.5=2025V1
```

`mu3e_checkout_branch` runs `git fetch origin <branch>` and `git reset --hard origin/<branch>` (then submodules). It overrides tag checkout.

Optional **`mu3e_checkout_merge`** / **`mu3e_checkout_merges`**: after the base checkout, run `git merge <hash>` for each commit (e.g. a PR tip). Composes with branch or tag:

```yaml
mu3e_tag: "dev-pr49"
mu3e_checkout_branch: "dev"
mu3e_checkout_merge: "75d8c48"
```

```tcsh
./runRereco -n -p -t dev-pr49 -b dev --config mu3e_checkout_merge=75d8c48 -g ...
```

If the hash is not yet in the local object database, fetch the PR ref first (Bitbucket example):

```tcsh
cd $MU3E_REREC_BASEDIR/mu3e-rereco_...
git fetch origin refs/pull-requests/49/from:pr-49
```

Then re-run Snakemake (or add the hash only after `git fetch` has made it reachable).

## Running

Host selection: `REREC_HOST=moor` or `./runRereco -H mu3edb0`

### MU3E setup only (no reprocessing)

Prepare the MU3E checkout, build, and relink without running any action. No `-m`/`-M` needed (omit run bounds so no tasks are expanded):

```tcsh
cd /path/to/mu3eanca/prompt/rereco

./runRereco -s rereco-v67 -j4 -p -t dev -b dev -g datav6.5=2025V1 mu3e_setup
```

Dry-run: add `-n`. Equivalent marker: `.markers/relink_bin_files.done`.

**Then run actions** — `midas_meta_all` / `trirec_all` need runs on the command line (`-m`/`-M`) or in config; `mu3e_setup` alone does not configure any:

```tcsh
./runRereco -s 260617-rereco -j4 -p -t dev -b dev -g datav6.5=2025V1 \
  -m 4756 -M 4756 midas_meta_all
```

### Full reprocessing

```tcsh
./runRereco -n -p -t v6.7 -g datav6.5=2025V1
./runRereco -j4 -p -t v6.7 -g datav6.5=2025V1
```

`-p` is a **Snakemake** flag (`--printshellcmds`): print each shell command before it runs. `-n` is Snakemake dry-run.

Single run / single action:

```tcsh
./runRereco -s rereco-v67 -j1 -p -m 4756 -M 4756 -t dev -b dev -g datav6.5=2025V1 midas_meta_all
./runRereco -s rereco-v67 -j1 -p -m 4756 -M 4756 -t dev -b dev -g datav6.5=2025V1 \
  .markers/midas_meta-run4756-midas_meta.done
```

From an existing setup directory:

```tcsh
cd $MU3E_REREC_BASEDIR/setups/rereco-v67
snakemake --directory $MU3E_REREC_BASEDIR/mu3e-rereco-v67 --cores 1 -p midas_meta_all
snakemake --directory $MU3E_REREC_BASEDIR/mu3e-rereco-v67 --cores 1 -p .markers/midas_meta-run4756-midas_meta.done
```

## Shared code with relval

MU3E checkout/build/relink and trirec live under `prompt/common/` (`mu3e_prepare.smk`, `mu3e_trirec.smk`, scripts). `runRereco` copies `common/` into each setup directory.

## Adding a new action later

1. Add a rule (in `prompt/common/` if reusable, or in `prompt/rereco/Snakefile`).
2. Extend `rereco_tasks` schema with a new `action` value.
3. Append that action’s outputs to `ALL_TARGETS` in the Snakefile.
4. Document the action in this README.

## Logs

- `logs/snakemake/run_mu3e_trirec-{task}.log`
- `logs/snakemake/run_midas_meta-{task}.log`
