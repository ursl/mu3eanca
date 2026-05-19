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
8. **Fill histograms** — `ana/bin/runFillHistograms` → `run/output/histograms-{scenario}.root`

If `compare_against_setup` is set (see below), three more steps run per scenario:

9. **RunCompare** — PDF summaries from histogram diff (`runCompareHistograms` on filled histogram ROOT files).
10. **Alignment treedump** — for each alignment object (`sensors`, `fibres`, `tiles`, `mppcs`), `mu3eTreeDumper` reads the **sort** files of the new and reference setups and writes compact treedump ROOT files under `run/output/` (e.g. `treedump-{scenario}-sensors.root`). This is extraction only, not a comparison yet.
11. **Histocompare** — the `histocompare` container compares each pair of treedump files (new vs reference) and writes PDFs (and related outputs) under `run/output/compare/`.

Snakemake implements steps 10–11 in a single rule named `run_histocompare` (treedump first, then container per object). Step 9 is the separate rule `run_compare_histograms`.

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

Edit `config.yaml` for machine-local paths and the scenario list. Each scenario has:

- `id` — wildcard name (e.g. `conf10_twolayer`)
- `sim_conf`, `trirec_conf` — paths under `run/`
- optional `trirec_conf_fallback`

Important keys:

- `mu3e_relval_basedir`, `relink_script`, `cdb_dbconn`, `cdb_GT`
- `relval_code_basedir` — path to this `prompt/relval` tree (for `ana/bin` tools)
- `compare_against_setup` — empty = no compare; otherwise the **setup name suffix** of the reference (see `runRelval --compare-against-setup`)

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
- The **histocompare** step (step 11 only) uses **podman** and `docker.io/mu3e/histocompare` with `--userns=keep-id` and `:Z` volume labels (write access on Linux servers with rootless podman / SELinux).
- Update `sim_scenarios` in `config.yaml` to add or remove physics configurations.
