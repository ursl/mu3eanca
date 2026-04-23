# MU3E RelVal Snakemake workflow

This workflow performs:

1. clone/update MU3E checkout for a selected tag
2. build MU3E in `_build`
3. run `relinkBinFiles`
4. run `mu3eSim`
5. run `mu3eSort`
6. run `mu3eTriRec`

for all configured scenario/geometry combinations.

## Current matrix

Defined in `config.yaml`:

- scenarios:
  - `cosmic` -> `sim_cosmic.json` -> trirec channel `cosmic`
  - `beam8` -> `bvr2026/sim_conf8.json` -> trirec channel `beam`
  - `beam18` -> `bvr2026/sim_conf18.json` -> trirec channel `beam`
- geometries:
  - `twolayer`
  - `threelayer`

This yields 6 branches (`3 scenarios x 2 geometries`) for sim/sort/trirec.

## Normal use cases
Run from mu3eanca/prompt/relval (this simply runs the default v6.5)
```bash
cd .../mu3eanca/prompt/relval
snakemake -j1 -p 
```

More realistic use cases:
```bash
cd .../mu3eanca/prompt/relval
snakemake -j1 -p --config mu3e_tag=v6.6pre0 cdb_GT=mcidealv6.6
snakemake -j1 -p --config mu3e_tag=v6.6pre0 cdb_GT=mcidealv6.6 cdb_dbconn=http://mu3edb0/cdb
```

Run from anywhere (recommended form?):
```bash
snakemake -j1 -p \
  -s "/Users/ursl/macros/ana/mu3eanca/prompt/relval/Snakefile" \
  --configfile "/Users/ursl/macros/ana/mu3eanca/prompt/relval/config.yaml"
```

### Use a specific MU3E tag from CLI

```bash
snakemake -j1 -p \
  -s "/Users/ursl/macros/ana/mu3eanca/prompt/relval/Snakefile" \
  --configfile "/Users/ursl/macros/ana/mu3eanca/prompt/relval/config.yaml" \
  --config mu3e_tag=v6.5
```

### Use a specific CDB global tag from CLI

```bash
snakemake -j1 -p \
  --config mu3e_tag=v6.6pre0 cdb_GT=mcidealv6.6
```

### Use a specific CDB dbconn from CLI

```bash
snakemake -j1 -p \
  --config mu3e_tag=v6.6pre0 cdb_dbconn=/Users/ursl/data/mu3e/cdb-v6.6
```

### Working directory behavior

The `Snakefile` sets:

- `workdir: <mu3e_relval_basedir>/<mu3e_dir>-<mu3e_tag>`

so `.snakemake` metadata and `.markers` are stored in that checkout automatically.
No manual `-d ...` is needed.

### Dry-run (show commands without executing)

```bash
snakemake -n -p \
  -s "/Users/ursl/macros/ana/mu3eanca/prompt/relval/Snakefile" \
  --configfile "/Users/ursl/macros/ana/mu3eanca/prompt/relval/config.yaml"
```

### Clean generated ROOT outputs and markers

```bash
snakemake clean -j1 -p \
  -s "/Users/ursl/macros/ana/mu3eanca/prompt/relval/Snakefile" \
  --configfile "/Users/ursl/macros/ana/mu3eanca/prompt/relval/config.yaml"
```

### Get summary

```bash
snakemake --summary  --config mu3e_tag=v6.6pre0
snakemake --detailed-summary  --config mu3e_tag=v6.6pre0
```


## Notes

- Checkout directory is computed as:
  - `<mu3e_relval_basedir>/<mu3e_dir>-<mu3e_tag>`
- `/` in tag names is replaced with `_` for directory safety.
- Update `config.yaml` to add/remove scenarios or geometries.
