# MU3E ReReco (Snakemake)

Re-run `mu3eTrirec` on existing sort ROOT files with a new MU3E checkout and/or CDB global tag.

Sibling to `prompt/relval/` — same setup pattern, but **no sim/sort**; inputs are pre-sorted data.

## What the workflow does

For each entry in `rereco_jobs` in `config.yaml`:

1. **Clone & prepare** — fetch/checkout the requested `mu3e_tag` (and submodules).
2. **Build** — `cmake` + `make` in the MU3E checkout `_build`.
3. **Relink** — `relinkBinFiles`; ensure `run/bvr2026` exists.
4. **TriRec** — `mu3eTrirec` on the configured sort input → `run/output/trirec-{job}.root`.

Default target: all `trirec-{job}.root` files.

## Layout on disk

| Setting | Meaning |
|--------|---------|
| `mu3e_rereco_basedir` | Root for workdirs and `setups/` (host config). |
| `mu3e_dir` | Prefix for checkout directory names (usually `mu3e`). |
| `sort_input_base` | Optional prefix for relative `sort_input` paths in job entries. |

After `./runRereco ...`:

```
$mu3e_rereco_basedir/
  setups/rereco_mu3e-v6.7_datav6.5=2025V1/   # copied Snakefile + merged config
  mu3e-rereco_mu3e-v6.7_datav6.5=2025V1/     # Snakemake workdir (MU3E checkout)
    run/output/trirec-{job}.root
```

## Configuration

Edit `config.yaml` (shared) and `config-<hostname>.yaml` (paths).

```yaml
rereco_jobs:
  - id: "run5700"
    run_id: 5700
    sort_input: "/data/mu3e/sort-run5700.root"
    trirec_conf: "trirec_twolayer_beam.conf"
    trirec_conf_fallback: "trirec.conf"
```

`sort_input` may be absolute, or relative to `sort_input_base`.

Override at runtime via Snakemake (after editing the setup `config.yaml`, or by re-running `runRereco`):

```tcsh
./runRereco -n -p --config mu3e_tag=v6.7 cdb_GT=datav6.5=2025V1
```

Host selection: `REREC_HOST=moor` or `./runRereco --host mu3edb0 ...`

## Quick start

```tcsh
cd /path/to/mu3eanca/prompt/rereco

# 1. Edit config.yaml: replace the example job with real sort paths.
# 2. Dry-run:
./runRereco -n -p --config mu3e_tag=v6.7 cdb_GT=datav6.5=2025V1

# 3. Run (4 cores):
./runRereco -j4 -p --config mu3e_tag=v6.7 cdb_GT=datav6.5=2025V1

# 4. Single job only:
cd $MU3E_REREC_BASEDIR/setups/rereco_mu3e-v6.7_...
snakemake --cores 1 -p run_mu3e_trirec --config run_id=5700
# (or use wildcards: run_mu3e_trirec --cores 1 -p --config ...  with target run5700 via rule name)
snakemake --cores 1 -p run/output/trirec-run5700.root
```

## Notes

- MU3E checkout/build rules mirror `prompt/relval/`; shared scripts can be factored out later.
- Trirec conf files are resolved under `$MU3E_DIR/run/` (same as relval).
- Logs: `logs/snakemake/run_mu3e_trirec-{job}.log` under the workdir.
