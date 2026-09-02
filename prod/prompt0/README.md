# prompt0 — 2026 prompt production

Perl workflow to bootstrap a dated production area and run a per-run
**pipeline of task scripts**. Successor to `run2025/scripts/processRuns`.
SLURM jobs still go through `slurm/run`.

```tcsh
cd /path/to/mu3eanca/prod/prompt0
```

---

## Layout

| Path | Role |
|------|------|
| `prompt` | CLI driver |
| `Prompt.pm` | Production-area setup/init |
| `Pipeline.pm` | Per-run task list, aliases, fail-fast |
| `TaskLib.pm` | Helpers for `tasks/*` (paths, RDB, ctx) |
| `tasks/` | One executable script per task |
| `prompt-base.cfg` | Shared defaults (pipeline, run_year, slurm wrappers, …) |
| `prompt-v7.1.cfg` | Version config (`base:` + workdir, GT, `[repo]` pins) |
| `host-merlin6.cfg` | merlin6 paths (`-H merlin6`; auto on `merlin-l-*`) |
| `host-moor.cfg` | Mac overlay (auto on `moor`) |

Shared clone/build: `../common/` (`ProdConfig`, `Setup`).

Host overlay is **`host-<platform>.cfg`**, not `config-<hostname>.cfg` as in startGT/relval. merlin6 is the platform; login nodes `merlin-l-001` / `merlin-l-002` map to it.

Production area:

```text
{setup_basedir}/{prompt_workdir}/
  mu3e/
  minalyzer/
  mutrig-offline-calibration/
  slurm/{prompt_workdir}.tar.gz
  slurm/jobs/{mlzr,trirec,sort}/
  githashes
```

`prompt_workdir` is **date-based** (`YYMMDD`), so a new cycle is a new directory, not a new release tag.

---

## Quick start

```tcsh
# 1) Edit host-merlin6.cfg paths if needed (once per platform).

# 2) Edit prompt-v7.1.cfg: prompt_workdir (today's YYMMDD) and [repo] pins.
#    Shared defaults live in prompt-base.cfg.

# 3) Dry-run first:
./prompt -H merlin6 -n list
./prompt -H merlin6 -n setup

# 4) On merlin6, with the usual software environment sourced:
./prompt setup
./prompt init
# or
./prompt bootstrap
```

`-s` overrides the dated workdir without editing the cfg:

```tcsh
./prompt -s 260915 bootstrap
```

---

## Targets

| Target | Description |
|--------|-------------|
| `setup` | Clone / checkout / build / relink `[repo]` packages (`-p` to restrict) |
| `init` | SLURM dirs, mlzr/trirec block dirs, `githashes`, alignment ROOT, tar for `slurm/run` |
| `bootstrap` | `setup` then `init` |
| `status` | Paths, binaries, tar |
| `list` | Resolved config + packages (default) |
| `run RUN ...` | Run the pipeline for each run (`-P` selects the list) |

Options:

```
-c FILE        config (default: prompt-v7.1.cfg)
-H PLATFORM    host-PLATFORM.cfg (default: hostname, with merlin-l-* → merlin6)
-s YYMMDD      override prompt_workdir
-p ID          only this [repo] id (setup; repeatable / comma-separated)
-P PIPE        pipeline alias or comma-separated tasks (default: cfg pipeline:)
-r RUNS        comma-separated run numbers
-j N           make -jN
-n             dry-run
```

---

## Per-run pipeline

Each run walks a comma-separated list of **task scripts** in `tasks/`. If a
task exits non-zero, the rest of the list is skipped for **that run** (the
next run still starts).

```
pipeline: beam
pipeline_alias: beam=midasmeta,skipSmallRuns,minalyzer,mu3esort,mu3etrirec
```

```tcsh
./prompt -n run 12345
./prompt -P beam run 12345
./prompt -P midasmeta,skipSmallRuns -n run 12345 12346
```

Exit codes from a task:

| Exit | Meaning |
|------|---------|
| 0 | Continue with the next task |
| 1 | Soft skip (e.g. `skipSmallRuns` when Events < `min_events`) |
| 2+ | Hard failure; `./prompt run` exits non-zero at the end |

`minalyzer` / `mu3esort` / `mu3etrirec` are stubs (dry-run logs the intent;
a real run exits 2 until ported from `processRuns`). `skipSmallRuns` and
`midasmeta` are live.

---

## Run year

Raw (and `{year}` in host `data_dir` / `raw_input_base`) follow the run number
in `prompt-base.cfg`:

```
run_year: 9410=2025
run_year: 2026
```

Runs `<= 9410` are 2025; `9411` and later are 2026 until you add the next bound.

---

## Adding a software package

In `prompt-v7.1.cfg` (or a date-copied cfg), add a `[repo]` block next to `mu3e`. `workdir` is the directory name under the production area. `id` can contain hyphens.

```
[repo]
id: some-tool
repo: "git@bitbucket.org:mu3e/some-tool"
checkout_branch: "dev"
workdir: some-tool
build: true
relink: false
submodules: false
# cmake_args: "-Dmu3e_DIR={mu3e}/install/{cmake_libdir}/cmake/mu3e"
```

If the batch job needs the package inside the tar (extracted by `slurm/run`):

```
tar_packages: some-tool
```

Prefixed keys also work (`minalyzer_repo:`, `minalyzer_checkout_branch:`, …) for ids that are `[A-Za-z0-9_]+`.

---

## Host overlay

`host-merlin6.cfg` (merlin6):

```
setup_basedir: "/data/experiment/mu3e/data/prod/prompt"
cmake_libdir: lib64
relink_script: "/psi/home/langenegger/mu3e/mu3eanca/perl/relinkBinFiles"
mu3eanca: "/psi/home/langenegger/mu3e/mu3eanca"
cdb_dbconn: "http://mu3edb0/cdb/"
data_dir: "/data/experiment/mu3e/data/{year}"
raw_input_base: "/data/experiment/mu3e/data/{year}/raw"
raw_input_layout: "runblock3"
slurm_run: "/psi/home/langenegger/mu3e/mu3eanca/slurm/run"
slurm_queue: "-p hourly"
```

Copy to `host-merlin7.cfg` when merlin7 paths are settled (`/data/project/mu3e/...`).
