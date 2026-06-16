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
| Bootstrap mu3eUtil | `midas_meta` | Clone/build `mu3eUtil` under `mu3e_rereco_basedir` |
| Clone & prepare mu3e | `trirec` | Checkout `mu3e_tag` (+ submodules) |
| Build mu3e | `trirec` | `cmake` + `make` in MU3E `_build` |
| Relink | `trirec` | `relinkBinFiles`; ensure `run/bvr2026` exists |

**Per task** (from `rereco_tasks` in `config.yaml`):

- **`trirec`** — `mu3eTrirec` on a sort file → `run/output/trirec-{task}.root`
- **`midas_meta`** — `mu3e_midas_meta` on every `*.mid.lz4` in `input_dir` → marker `.markers/midas_meta-{task}.done` (ROOT meta files are written by the tool next to the MID inputs)

Default target (`rule all`): all outputs/markers for every configured task.

## Layout on disk

| Setting | Meaning |
|--------|---------|
| `mu3e_rereco_basedir` | Root for workdirs, `setups/`, shared `mu3eUtil` clone |
| `mu3e_dir` | Prefix for MU3E checkout directory names |
| `sort_input_base` | Optional prefix for relative `sort_input` / `input_dir` paths |

After `./runRereco ...`:

```
$mu3e_rereco_basedir/
  mu3eUtil/_build/tools/midasMeta/mu3e_midas_meta   # shared util build
  setups/rereco_mu3e-v6.7_datav6.5=2025V1/          # copied Snakefile + common/
  mu3e-rereco_mu3e-v6.7_datav6.5=2025V1/            # Snakemake workdir (MU3E checkout when trirec runs)
    run/output/trirec-{task}.root
    .markers/midas_meta-{task}.done
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

Template placeholders: `{run}` (plain number), `{run:05d}` (zero-padded). Optional `id_tpl` defaults to `run{run}-{action}`.

When `run_list_file` is set, explicit `rereco_tasks` are ignored.

CLI (written into setup `config.yaml`):

```tcsh
./runRereco -n -p --minRun 5700 --maxRun 5800 --config mu3e_tag=v6.7 cdb_GT=datav6.5=2025V1
```

Same via Snakemake config override:

```tcsh
./runRereco -n -p --config min_run=5700 max_run=5800 mu3e_tag=v6.7 cdb_GT=...
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

```yaml
  - action: midas_meta
    input_file_tpl: "/data/raw/run{run:05d}.mid.lz4"
```

Or a whole directory (all `*.mid.lz4` in that dir):

```yaml
  - id: "run5700-midas"
    action: midas_meta
    input_dir: "/data/raw/run5700"
```

Single-file mode runs one `mu3e_midas_meta` invocation; directory mode processes every `*.mid.lz4` (same pattern as `run2025/scripts/slurm-midasMeta.csh`). Requires mu3eUtil bootstrap only — no MU3E checkout unless you also have `trirec` tasks.

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
./runRereco -n -p --config mu3e_tag=dev mu3e_checkout_branch=dev cdb_GT=datav6.5=2025V1
```

`mu3e_checkout_branch` runs `git fetch origin <branch>` and `git reset --hard origin/<branch>` (then submodules). It overrides tag checkout.

Optional **`mu3e_checkout_merge`** / **`mu3e_checkout_merges`**: after the base checkout, run `git merge <hash>` for each commit (e.g. a PR tip). Composes with branch or tag:

```yaml
mu3e_tag: "dev-pr49"
mu3e_checkout_branch: "dev"
mu3e_checkout_merge: "75d8c48"
```

```tcsh
./runRereco -n -p --config mu3e_tag=dev-pr49 mu3e_checkout_branch=dev mu3e_checkout_merge=75d8c48 cdb_GT=...
```

If the hash is not yet in the local object database, fetch the PR ref first (Bitbucket example):

```tcsh
cd $MU3E_REREC_BASEDIR/mu3e-rereco_...
git fetch origin refs/pull-requests/49/from:pr-49
```

Then re-run Snakemake (or add the hash only after `git fetch` has made it reachable).

## Running

Host selection: `REREC_HOST=moor` or `./runRereco --host mu3edb0`

```tcsh
cd /path/to/mu3eanca/prompt/rereco

./runRereco -n -p --config mu3e_tag=v6.7 cdb_GT=datav6.5=2025V1
./runRereco -j4 -p --config mu3e_tag=v6.7 cdb_GT=datav6.5=2025V1
```

Single task / action:

```tcsh
cd $MU3E_REREC_BASEDIR/setups/rereco_mu3e-v6.7_...
snakemake --cores 1 -p run/output/trirec-run5700-trirec.root
snakemake --cores 1 -p .markers/midas_meta-run5700-midas.done
snakemake --cores 1 -p run_midas_meta --config ...   # wildcard task=run5700-midas
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
