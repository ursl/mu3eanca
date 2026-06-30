# MU3E RelVal (Snakemake)

Automated release validation: check out a MU3E tag, build, run simulation or data reprocessing tasks, fill validation histograms, and optionally compare against a reference setup.

## Workflow overview

1. **`initRelval`** — once per setup: write `setups/<name>/config.yaml`, copy Snakefile + `common/`.
2. **`snakemake mu3e_setup`** — clone/checkout MU3E, build, relink.
3. **`snakemake relval_all`** (or `compare_all` / `histocompare_all`) — run configured tasks.

Like `prod/rereco/`, setup parameters live in `setups/<name>/config.yaml`; operational overrides (`compare_against_setup`, …) via `--config` on the snakemake command line.

## What the workflow does

For each entry in **`relval_tasks`** (see `config.yaml`):

| `mode` | Pipeline |
|--------|----------|
| `sim` | mu3eSim → mu3eSort → mu3eTrirec → fill histograms → treedump → [compare, histocompare] |
| `data` | existing `sort_input` → mu3eTrirec → fill histograms → treedump → [compare, histocompare] |

Shared steps (once per setup):

1. **Bootstrap** — clone/build `mu3eUtil` and `mu3eValidation` under `mu3e_relval_basedir`.
2. **Clone & prepare / build / relink** — `mu3e_setup` target.

Each task has its own **`cdb_GT`**, **`trirec_conf`**, and (for sim) **`sim_conf`**. Example tasks in the default config:

| id | mode | GT (example) | trirec conf |
|----|------|--------------|-------------|
| `signal` | sim | `mcidealv6.9` | `trirec.conf` |
| `conf8` | sim | `mcrealisticv6.9=2025V0` | `trirec_twolayer_beam.conf` |
| `run06232` | data | `datav6.9=2025V0` | `trirec_twolayer_beam.conf` |

If `compare_against_setup` is set (reference setup name, e.g. `relval-v6.8`):

- **`run_compare_histograms`** — PDF summaries from histogram diff.
- **`run_histocompare`** — Docker histocompare on treedump pairs (new vs reference).

### Snakemake targets

| Target | Meaning |
|--------|---------|
| `mu3e_setup` | Clone, build, relink — **run first** |
| `relval_all` | Full pipeline for all tasks (default `all`) |
| `compare_all` | All `run_compare_histograms` markers (needs `compare_against_setup`) |
| `histocompare_all` | All histocompare markers |

## Running

### 1. Create setup (once)

```tcsh
cd /path/to/mu3eanca/prod/relval

./initRelval -s relval-v6.9pre -t v6.9pre

# optional overrides:
./initRelval -s relval-v6.9pre -t v6.9pre --config compare_against_setup=relval-v6.8
```

**MU3E checkout** (pick one base, then optional merges):

| Goal | `initRelval` |
|------|----------------|
| Release tag | `-t v6.9pre` (no `-b`) → `git checkout v6.9pre` |
| Branch HEAD | `-t <label> -b dev` → `git checkout dev` + reset to `origin/dev` |
| PR on top of `dev` | `-b dev` + `--config mu3e_checkout_merge=<commit>` |
| PR in mu3eUtil | add `--config mu3eUtil_checkout_merge=<commit>` |

Example before merging a PR (mu3e + mu3eUtil):

```tcsh
./initRelval -s relval-dev-pr49 -t dev-pr49 -b dev \
  --config mu3e_checkout_merge=75d8c48 \
  --config mu3eUtil_checkout_merge=a1b2c3d4
```

`-t` is always required (setup label / `mu3e_tag` in outputs). With `-b dev`, git uses the branch, not the tag. Merges run after checkout (`mu3e`) and after `submodule update` (`modules/mu3eUtil`). If the commit is not local yet, fetch the PR branch first (see `clone_and_prepare_mu3e` error hint or `prod/rereco/README.md`).

Re-run `initRelval` to refresh `Snakefile`/`common/` and patch setup config.

Set **`sort_input_base`** in `config-<host>.yaml` for data tasks (e.g. `run06232`).

### 2. MU3E setup

```tcsh
set WF = /path/to/relval/setups/relval-v6.9pre
cd $WF

snakemake --cores 4 -p mu3e_setup
```

### 3. Full relval (with compare)

```tcsh
cd $WF

snakemake --cores 4 -p --config compare_against_setup=relval-v6.8 -- relval_all
```

Compare/histocompare only:

```tcsh
snakemake --cores 4 -p --config compare_against_setup=relval-v6.8 -- compare_all
snakemake --cores 4 -p --config compare_against_setup=relval-v6.8 -- histocompare_all
```

### Legacy: `runRelval`

`runRelval` still init+runs in one step (creates `setups/relval_<tag>/`, runs snakemake). Prefer `initRelval` + direct snakemake for rereco-style workflows.

---

## Layout on disk

Configured in `config.yaml`:

| Setting | Meaning |
|--------|---------|
| `mu3e_relval_basedir` | Root for all relval data (workdirs, `setups/`, bootstrap clones). |
| `mu3e_dir` | Prefix for checkout directory names (usually `mu3e`). |
| `mu3e_tag` | Setup label and default git tag (overridable on CLI). |
| `mu3e_checkout_branch` | If set (e.g. `dev`), track `origin/<branch>` HEAD instead of a tag. |

**Checkout / workdir** (Snakemake `workdir`):

```text
<mu3e_relval_basedir>/<mu3e_workdir_name>/
```

`initRelval` / `runRelval` set `mu3e_workdir_name` to `mu3e-<setup-name>` (e.g. `mu3e-relval-v6.9pre`).

**Frozen setup** (copy of Snakefile + patched config for reproducibility):

```text
<mu3e_relval_basedir>/setups/<setup-name>/
```

**Status exports** (written by `runRelval` after a run):

```text
<workdir>/status/summary.tsv
<workdir>/status/detailed-summary.tsv
```

**Compare outputs** (when enabled):

```text
<workdir>/run/output/treedump-<scenario>-<object>.root   # from mu3eTreeDumper (step 10)
<workdir>/run/output/compare/<scenario>__<this>__vs__<reference>/
    histocompare-<scenario>-<object>.pdf                   # from histocompare container (step 11)
    summary-*.pdf                                          # from runCompareHistograms (step 9)
```

## Configuration

**Shared defaults:** `config.yaml` (scenarios, tags, output templates — same on all machines).

**Host overrides:** `config-<hostname>.yaml` (paths only). Snakemake loads it automatically when the file exists:

| File | When used |
|------|-----------|
| `config-moor.yaml` | hostname `moor` (Mac localhost) |
| `config-mu3edb0.yaml` | hostname `mu3edb0` (Ubuntu server) |

Resolution order: `config.yaml`, then `config-<host>.yaml` (later keys win). Host is:

1. `RELVAL_HOST` environment variable, or
2. short hostname (`moor`, `mu3edb0`, …), or
3. `./runRelval --host moor` / `--host mu3edb0`

`runRelval` merges base + host config into the frozen `setups/.../config.yaml` for that run.

Add a new machine by copying a host file and editing four paths:

- `mu3e_relval_basedir`, `relink_script`, `cdb_dbconn`, `relval_code_basedir`

Each task in **`relval_tasks`** has:

- `id` — task name (wildcard, e.g. `signal`, `conf8`, `run06232`)
- `mode` — `sim` or `data`
- `cdb_GT` — global tag for sort/trirec on this task
- `trirec_conf` — path under `run/`
- `sim_conf` — required for `mode: sim`
- `sort_input` — required for `mode: data` (absolute or relative to `sort_input_base`)
- optional `run_id`, `n_events`, `trirec_conf_fallback`

Legacy **`sim_scenarios`** is still supported when `relval_tasks` is empty.

Other important keys:

- `cdb_GT`, `compare_against_setup` — can still be overridden on the CLI via `--config`

Build histogram tools once (or after code changes):

```bash
cd ana && make
```

## Recommended: `runRelval`

Wrapper that creates/updates a setup under `setups/`, runs Snakemake, and exports status TSVs.

```bash
cd prod/relval
./runRelval -j4 -p --config mu3e_tag=v6.5 cdb_GT=mcidealv6.5
```

Required on the command line: `mu3e_tag=...` (via `--config`). Optional overrides: `cdb_GT=...`, `cdb_dbconn=...`, `mu3e_checkout_branch=dev` (branch HEAD instead of tag).

Example on `dev`:

```tcsh
./runRelval -j4 -p --config mu3e_tag=dev mu3e_checkout_branch=dev cdb_GT=mcidealv6.5
```

With a PR commit merged on top of `dev`:

```tcsh
./runRelval -j4 -p --config mu3e_tag=dev-pr49 mu3e_checkout_branch=dev mu3e_checkout_merge=75d8c48 cdb_GT=mcidealv6.5
```

(`mu3e_checkout_merge` / `mu3e_checkout_merges` run `git merge <hash>` after checkout; see `prod/rereco/README.md` for fetch notes.)

| Option | Purpose |
|--------|---------|
| `--setup-name NAME` | Frozen setup folder name; default `relval_<mu3e_dir>-<tag>_<cdb_GT>` |
| `--host HOST` | Force host config file `config-<HOST>.yaml` (default: hostname or `RELVAL_HOST`) |
| `--compare-against-setup NAME` | Reference setup name (same as `compare_against_setup` in config); enables compare rules |
| `--all` | Run a predefined chain of releases (project-specific) |
| `--dry-run` | With `--all`, print commands only |

Examples:

```bash
# Baseline setup (no histogram compare)
./runRelval -j4 -p --config mu3e_tag=v6.3.3 cdb_GT=mcidealv6.1 cdb_dbconn=http://mu3edb0/cdb

# Compare new release against an existing setup
./runRelval -j4 -p \
  --compare-against-setup relval_mu3e-v6.3.3_mcidealv6.1 \
  --config mu3e_tag=v6.4.4 cdb_GT=mcidealv6.5 cdb_dbconn=http://mu3edb0/cdb
```

On Debian/Ubuntu Snakemake 6.x, `runRelval` rewrites `-jN` to `--cores N` so `--summary` works after the run.

### When a rule fails (logs)

Snakemake’s main log (`setups/.../.snakemake/log/...`) often only says `Error in rule …` without the shell output. Use:

1. **Per-rule logs** (stdout+stderr from the shell), under the MU3E workdir, e.g.  
   `logs/snakemake/run_histocompare-conf10_twolayer.log`  
   For histocompare, each alignment object also writes  
   `run/output/compare/.../histocompare-<scenario>-<obj>.docker.log`.

2. **`--show-failed-logs`** (Snakemake 6+) — prints failed job logs to the terminal after the run:  
   `./runRelval --show-failed-logs -j4 -p --config ...`

3. **Job metadata** in `setups/<setup>/.snakemake/log/<timestamp>/` (cluster/job scripts if present).

4. Re-run one job with print:  
   `snakemake --cores 1 -p run_histocompare --config ...`  
   (add scenario wildcard if needed via target file)

## Direct Snakemake use

From `prod/relval` (uses local `config.yaml` and default workdir naming):

```bash
snakemake -j4 -p --config mu3e_tag=v6.6 cdb_GT=mcidealv6.5
```

Dry-run:

```bash
snakemake -n -p --config mu3e_tag=v6.6
```

Clean outputs and markers in the active workdir:

```bash
snakemake clean -j1 -p --config mu3e_tag=v6.6
```

Summary only:

```bash
snakemake --summary --cores 4 --config mu3e_tag=v6.6
snakemake --detailed-summary --cores 4 --config mu3e_tag=v6.6
```

## Web dashboard (`db1/rest`)

The UI is **not** a separate server under `prod/relval`; it lives in **`db1/rest`**:

- Logic: `db1/rest/lib/relvalCore.mjs`, `db1/rest/lib/relvalRouter.mjs`
- Static UI: `db1/rest/public/relval/`

It scans `RELVAL_BASEDIR` for directories named `mu3e-*`, reads each `status/summary.tsv`, lists compare PDFs, and serves files under that tree.

### Configure per machine

Set **`RELVAL_BASEDIR`** to the **same absolute path** as `mu3e_relval_basedir` in `config.yaml`:

```bash
cd db1/rest
cp .env.example .env
# RELVAL_BASEDIR=/mnt/data2/relval
npm start
```

Open **`http://<host>:<PORT>/relval/`** (trailing slash matters; `/relval` redirects).

The table shows only setups that have a **reference release** from compare output (rows with reference `n/a` are hidden).

Standalone dashboard (same code, default port 8787):

```bash
cd db1/rest
RELVAL_BASEDIR=/path/to/relval npm run relval-dashboard
```

If `RELVAL_BASEDIR` is unset, the page loads but the API returns 503 with a short hint.

## Notes

- Snakemake metadata (`.snakemake`, `markers`) lives inside the MU3E workdir; no manual `-d` is required.
- Alignment treedumps use `mu3eUtil`’s `mu3eTreeDumper` and config from `mu3eValidation/scripts/treedump_and_histocompare/config.json`.
- **Supported hosts:** macOS and Linux (Ubuntu on mu3edb0, etc.). Histocompare uses Docker with the same command on both.

### Histocompare (Docker)

Step 11 (`run_histocompare`) runs `docker.io/mu3e/histocompare` in Docker. It writes to a **host `mktemp` directory** bind-mounted at `/workdir`, then copies PDF/ROOT/log into `run/output/compare/`.

- **Linux:** container runs as your user (`--user $(id -u):$(id -g)`) so outputs on NFS are owned correctly; scratch dir uses `$TMPDIR` or `/tmp`.
- **macOS** (Docker Desktop / Colima): `--user` is omitted; scratch dir is **`$MU3E_RELVAL_BASEDIR/.relval-histocompare.*`** (Colima cannot write bind mounts from `/tmp` or Snakemake’s `$TMPDIR` under `/var/folders/…`). On Apple Silicon, `--platform linux/amd64` is added (image is amd64-only).

**macOS** (Docker Desktop or Colima):

```bash
colima start          # if using Colima
docker pull docker.io/mu3e/histocompare
```

**Ubuntu / Linux:**

```bash
docker pull docker.io/mu3e/histocompare
# If "permission denied" on the socket, add your user to the docker group and re-login:
# sudo usermod -aG docker $USER
```

If histocompare fails with exit **125**, check `run/output/compare/.../*.docker.log` — usually the Docker daemon is not running or not reachable.

Optional overrides in host `config-*.yaml`:

```yaml
histocompare_docker_cmd: docker   # default
histocompare_image: docker.io/mu3e/histocompare
histocompare_docker_user: auto     # auto | host | none
histocompare_docker_platform: auto  # auto | linux/amd64 | ""
```

To test container write access (override entrypoint):

```bash
COMPARE_DIR=".../run/output/compare/<scenario-dir>"
# Linux:
docker run --rm --user "$(id -u):$(id -g)" --entrypoint sh \
  -w /workdir -v "$COMPARE_DIR:/workdir" \
  docker.io/mu3e/histocompare -c 'touch _write_test && echo OK'
# macOS/Colima (no --user):
docker run --rm --entrypoint sh \
  -w /workdir -v "$COMPARE_DIR:/workdir" \
  docker.io/mu3e/histocompare -c 'touch _write_test && echo OK'
ls -l "$COMPARE_DIR/_write_test"
```

**Cleanup** if files were created with wrong ownership from an earlier container run:

```bash
sudo chown -R $(id -un):$(id -gn) /path/to/relval/
```
- Update `sim_scenarios` in `config.yaml` to add or remove physics configurations.
