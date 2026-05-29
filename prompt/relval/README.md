# MU3E RelVal (Snakemake)

Automated release validation: check out a MU3E tag, build, run simulation/reconstruction for configured scenarios, fill validation histograms, and optionally compare against a reference setup.

## What the workflow does

For each entry in `sim_scenarios` in `config.yaml`, Snakemake runs:

1. **Bootstrap** — clone/build `mu3eUtil` and `mu3eValidation` under `mu3e_relval_basedir` (once).
2. **Clone & prepare** — fetch/checkout the requested `mu3e_tag` (and submodules).
3. **Build** — `cmake` + `make` in the MU3E checkout `_build`.
4. **Relink very large bin files** — `relinkBinFiles`; ensure `run/bvr2026` exists.
5. **Sim** — `mu3eSim` → `run/output/sim-{scenario}.root`
6. **Sort** — `mu3eSort` (uses `cdb_dbconn`, `cdb_GT`)
7. **TriRec** — `mu3eTrirec` → `run/output/trirec-{scenario}.root`
8. **Fill histograms** — `ana/bin/runFillHistograms` → `run/output/histograms-{scenario}.root` (from trirec).
9. **Treedump** — rule `run_treedump`: after **sort**, `mu3eTreeDumper` writes `run/output/treedump-{scenario}-{object}.root` for each alignment object (`sensors`, `fibres`, `tiles`, `mppcs`). Always part of the default targets (not tied to compare mode). Runs in parallel with trirec / fill-hist once sort exists.

If `compare_against_setup` is set (see below), two more steps run per scenario:

10. **RunCompare** — PDF summaries from histogram diff (`runCompareHistograms` on filled histogram ROOT files).
11. **Histocompare** — rule `run_histocompare`: `histocompare` container compares treedump pairs (new vs reference) → PDFs under `run/output/compare/`. Reference treedumps must already exist from a **previous relval run** of the reference setup (same `run_treedump` rule).

Default targets are the histogram ROOT files; with compare enabled, compare/histocompare marker files are added.

## Layout on disk

Configured in `config.yaml`:

| Setting | Meaning |
|--------|---------|
| `mu3e_relval_basedir` | Root for all relval data (workdirs, `setups/`, bootstrap clones). |
| `mu3e_dir` | Prefix for checkout directory names (usually `mu3e`). |
| `mu3e_tag` | Default git tag (overridable on CLI). |

**Checkout / workdir** (Snakemake `workdir`):

```text
<mu3e_relval_basedir>/<mu3e_workdir_name>/
```

`runRelval` sets `mu3e_workdir_name` to `mu3e-<setup-name>` (e.g. `mu3e-relval_mu3e-v6.5_mcidealv6.5`). Slashes in tags are replaced by `_` in directory names.

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

Each scenario in `config.yaml` has:

- `id` — wildcard name (e.g. `conf10_twolayer`)
- `sim_conf`, `trirec_conf` — paths under `run/`
- optional `trirec_conf_fallback`

Other important keys:

- `cdb_GT`, `compare_against_setup` — can still be overridden on the CLI via `--config`

Build histogram tools once (or after code changes):

```bash
cd ana && make
```

## Recommended: `runRelval`

Wrapper that creates/updates a setup under `setups/`, runs Snakemake, and exports status TSVs.

```bash
cd prompt/relval
./runRelval -j4 -p --config mu3e_tag=v6.5 cdb_GT=mcidealv6.5
```

Required on the command line: `mu3e_tag=...` (via `--config`). Optional overrides: `cdb_GT=...`, `cdb_dbconn=...`.

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
   `run/output/compare/.../histocompare-<scenario>-<obj>.podman.log`.

2. **`--show-failed-logs`** (Snakemake 6+) — prints failed job logs to the terminal after the run:  
   `./runRelval --show-failed-logs -j4 -p --config ...`

3. **Job metadata** in `setups/<setup>/.snakemake/log/<timestamp>/` (cluster/job scripts if present).

4. Re-run one job with print:  
   `snakemake --cores 1 -p run_histocompare --config ...`  
   (add scenario wildcard if needed via target file)

## Direct Snakemake use

From `prompt/relval` (uses local `config.yaml` and default workdir naming):

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

The UI is **not** a separate server under `prompt/relval`; it lives in **`db1/rest`**:

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

- Snakemake metadata (`.snakemake`, `.markers`) lives inside the MU3E workdir; no manual `-d` is required.
- Alignment treedumps use `mu3eUtil`’s `mu3eTreeDumper` and config from `mu3eValidation/scripts/treedump_and_histocompare/config.json`.
- **Supported hosts:** macOS (local dev) and Linux (Ubuntu on mu3edb0, etc.). The Snakefile picks Podman mount/user options from the OS (`:Z` and `--userns=keep-id` on Linux only).

### Histocompare (Podman)

Step 11 (`run_histocompare`) runs `docker.io/mu3e/histocompare` in Podman. It writes to a **host `mktemp` directory**, then copies PDF/ROOT/log into `run/output/compare/`. Do not use `:U` on mounts — on NFS it can leave files owned by a podman subuid.

**macOS** (Podman machine must be running before relval):

```bash
podman machine start          # once per login/reboot
podman pull docker.io/mu3e/histocompare
```

If histocompare fails with exit **125**, check `run/output/compare/.../*.podman.log` — usually “Cannot connect to Podman” means the machine is stopped.

**Ubuntu / Linux** (rootless podman):

```bash
systemctl --user start podman.socket   # if needed
podman pull docker.io/mu3e/histocompare
```

On Linux/NFS, histocompare writes to tmpfs `/workdir` and **`podman cp` runs while the container is still up** (tmpfs is gone after exit). On macOS, a host `mktemp` dir is bind-mounted at `/workdir`. Container cwd is `/tmp` on both.

To test container write access on **Linux** (override entrypoint; check owner is you):

```bash
COMPARE_DIR=".../run/output/compare/<scenario-dir>"
podman run --rm --userns=keep-id --entrypoint sh \
  -w /workdir -v "$COMPARE_DIR:/workdir:Z" \
  docker.io/mu3e/histocompare -c 'touch _write_test && echo OK'
ls -l "$COMPARE_DIR/_write_test"   # should show your user, e.g. mu3e, not 297608
```

**Cleanup** if files were already created with wrong ownership (`297608` etc. on NFS):

```bash
sudo chown -R mu3e:users /mnt/data2/relval/
```
- Update `sim_scenarios` in `config.yaml` to add or remove physics configurations.
