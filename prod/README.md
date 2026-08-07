# MU3E prod (Perl relval)

Perl replacement for the legacy `snakemake/relval` workflow: **setup** MU3E (and helpers) under a shared basedir, **run** configured sim/data tasks, then **compare** a new release against the previous one.

Host-specific paths live in `config-<hostname>.cfg` (e.g. `config-moor.cfg`): `setup_basedir`, `cdb_dbconn`, `raw_input_base`, `relink_script`, …

```tcsh
cd /Users/ursl/macros/ana/mu3eanca/prod
```

---

## 1. Create a versioned setup config

For each MU3E tag/branch you want on disk, add a small setup file:

```text
config-setup-v6.5.cfg
config-setup-v6.9pre0.cfg
config-setup-vX.Y.cfg      # copy from an existing one
```

Minimal contents:

```
target: setup

mu3e_repo: "git@bitbucket.org:mu3e/mu3e"
mu3e_tag: "vX.Y"
# mu3e_checkout_branch: "dev"          # optional instead of tag
# mu3e_merges: "some-feature-branch"   # optional merge after checkout
make_jobs: 10
```

Default checkout path:

```text
<setup_basedir>/mu3e-vX.Y/
```

Helpers (once per basedir, or when you need a rebuild):

```text
config-setup-mu3eUtil.cfg
config-setup-mu3eValidation.cfg
config-setup-all.cfg          # include: several setup configs
```

---

## 2. Create a versioned relval config

For each release you will **run** tasks against, copy a relval config and point it at that workdir:

```text
config-relval-v6.5.cfg
config-relval.cfg             # typically the “current” / newest workdir
config-relval-vX.Y.cfg
config-relval-all.cfg         # include: several relval configs (like setup-all)
```

Important keys:

```
target: all

mu3e_workdir: "mu3e-vX.Y"
mu3e_tag: "vX.Y"

# tasks: [task] blocks (signal, conf8, run06232, …)
```

Keep the `[task]` list consistent across versions you intend to compare (same `id`s).

---

## 3. Run setup

Clone / checkout / build / relink:

```tcsh
# one MU3E version
./relval -c config-setup-v6.9pre0.cfg

# dry-run first
./relval -c config-setup-v6.9pre0.cfg -n

# util + validation + current MU3E (see includes in the file)
./relval -c config-setup-all.cfg
```

Check status:

```tcsh
./relval -c config-setup-v6.9pre0.cfg status
```

---

## 4. Run relval (sim / sort / trirec / …)

Against the workdir named in the relval config:

```tcsh
# current / newest (e.g. mu3e-v6.9pre0)
./relval -c config-relval.cfg

# all configured versions (see includes in config-relval-all.cfg)
./relval -c config-relval-all.cfg

# a specific older versioned config
./relval -c config-relval-v6.5.cfg

# dry-run
./relval -c config-relval.cfg -n
```

CLI overrides (no edit of the cfg file):

```tcsh
./relval -c config-relval.cfg mu3e_workdir=mu3e-v6.5
# or
./relval -c config-relval.cfg -s mu3e-v6.5
```

Outputs land under:

```text
<setup_basedir>/<mu3e_workdir>/run/output/
  sim-*.root  sort-*.root  trirec-*.root  histograms-*.root  treedump-*-*.root
```

Pipeline per task:

| `mode` | Steps |
|--------|--------|
| `sim`  | mu3eSim → mu3eSort → mu3eTrirec → fillhist → treedump |
| `data` | raw MID → mu3eSort → mu3eTrirec → fillhist |

---

## 5. Compare new vs most recent old

**Prerequisite:** both workdirs have been run with `target: all` (same task ids), so histogram ROOT files exist on both sides.

Example: new = `mu3e-v6.9pre0`, previous = `mu3e-v6.5`.

```tcsh
# 1) run / confirm old
./relval -c config-relval-v6.5.cfg

# 2) run new
./relval -c config-relval.cfg

# 3) compare (new workdir from config-relval.cfg vs old)
./relval -c config-relval.cfg compare compare_against_setup=mu3e-v6.5
```

`compare_against_setup` may be `mu3e-v6.5` or `v6.5` (auto-prefixed with `mu3e-` if needed).

Or set it permanently in the new config:

```
compare_against_setup: "mu3e-v6.5"
```

then:

```tcsh
./relval -c config-relval.cfg compare
```

Compare PDFs:

```text
<setup_basedir>/mu3e-v6.9pre0/run/output/compare/<task>__mu3e-v6.9pre0__vs__mu3e-v6.5/
```

Docker **histocompare** is not wired in this Perl driver yet; use target `compare` for the ROOT/PDF histogram diffs.

---

## Targets and options (summary)

| Target | Meaning |
|--------|---------|
| `setup` | clone/checkout/merge/[submodules]/[build]/[relink] |
| `status` | show workdir / HEAD |
| `all` | run all `[task]` pipelines |
| `compare` | PDF histogram compare vs `compare_against_setup` |
| `list` | print resolved config |

| Option | Meaning |
|--------|---------|
| `-c FILE` | config file |
| `-H HOST` | force `config-HOST.cfg` |
| `-s NAME` | workdir override |
| `-n` | dry-run |
| `-j N` | make jobs (setup build) |
| `key=value` | CLI override (e.g. `mu3e_workdir=…`, `compare_against_setup=…`) |

Target resolution: **CLI target** → config `target:` → `list`.

---

## Layout on disk

Configured by `setup_basedir` in the host config:

```text
<setup_basedir>/
  mu3eUtil-dev/                 # from config-setup-mu3eUtil.cfg
  mu3eValidation-master/        # from config-setup-mu3eValidation.cfg
  mu3e-v6.5/                    # from config-setup-v6.5.cfg
  mu3e-v6.9pre0/                # from config-setup-v6.9pre0.cfg
    _build/
    run/output/
```

Legacy Snakemake trees remain under `snakemake/` and are not required for the flow above (histogram tools are still taken from `snakemake/relval/ana/bin/` for now).
