# startGT — local test-CDB bootstrap

Perl workflow to build a **local test conditions database (test-CDB)** with global tags for MU3E analysis: `mcideal`, `mcrealistic`, and `data`.

It replaces ad-hoc `cdbInitGT` usage for development and relval-style checks. One mcideal `mu3eSim` run produces a single alignment ROOT file; all GT flavours read that geometry. Installed-component selection (2025 vs 2026 vs full ideal) is handled inside `cdbRunPayloadWriter`.

```tcsh
cd /path/to/mu3eanca/prod/startGT
```

---

## Quick start

```tcsh
# 1) Host overlay (once): copy config-moor.cfg or config-mu3edb0.cfg → config-<hostname>.cfg
#    Set setup_basedir and CDB_code_basedir.

# 2) Build CDB tools (once, or after C++ changes in db0/cdb2):
cd /path/to/mu3eanca/db0/cdb2
make lib/libCDB.so bin/cdbRunPayloadWriter

# 3) Bootstrap mu3e + test-CDB for a release:
cd /path/to/mu3eanca/prod/startGT
./startGT -c GT-v6.9.cfg setup
./startGT -c GT-v6.9.cfg sim
./startGT -c GT-v6.9.cfg -g mcideal alltags
./startGT -c GT-v6.9.cfg -g data alltags
```

Default config is `GT-v7.1.cfg`; use `-c GT-v6.9.cfg` for v6.9.

Dry-run (print commands, do not execute):

```tcsh
./startGT -c GT-v6.9.cfg -g mcideal -n alltags
```

---

## Layout

| Path | Role |
|------|------|
| `startGT` | CLI driver |
| `common/StartGT.pm` | Core logic (payload writers, tag naming, global-tag merge) |
| `GT-v6.9.cfg`, `GT-v7.1.cfg` | Release configs (`alltags` list, mu3e tag, conditions year) |
| `config-<hostname>.cfg` | Host overlay (`setup_basedir`, `CDB_code_basedir`, …) |

Shared infrastructure comes from `../relval/common/` (`RelvalConfig`, `Setup`).

---

## Targets

| Target | `-g` required | Description |
|--------|---------------|-------------|
| `setup` | no | Clone/checkout/build/relink mu3e under `setup_basedir` |
| `sim` | no | One mcideal `mu3eSim` run → alignment ROOT (shared by all GTs) |
| `alignment` | yes | Write alignment payloads only |
| `quality` | yes | Write perfect quality payloads (IOV 1) |
| `alltags` | yes | All calibrations listed in config `alltags:` (alias: `payloads`) |
| `status` | optional | Paths, build state, tag presence (`-g` shows GT-specific detail) |
| `list` | no | Print resolved config (default if no target given) |

Options:

```
-c FILE   config file (default: GT-v7.1.cfg)
-H HOST   host overlay (config-HOST.cfg; default: machine hostname)
-j N      make -jN for mu3e build
-n        dry-run
-g FLAV   GT flavour: mcideal | mcrealistic | data
-h        help
```

---

## Global tag naming

`-g` is a **flavour**; the full GT name is built from the config:

| Flavour | GT name (example v6.9) |
|---------|-------------------------|
| `mcideal` | `mcidealv6.9` |
| `mcrealistic` | `mcrealisticv6.9=2025V0` |
| `data` | `datav6.9=2025V0` |

Config keys: `gt_base`, `conditions_year`, `conditions_tag`.

For v7.1, `conditions_year` is `2026` → `mcrealisticv7.1=2026V0`, `datav7.1=2026V0`.

---

## `alltags` calibrations

The `alltags:` list in the GT config drives what `alltags` writes. Order is preserved during payload creation; the **global tag JSON is sorted alphabetically** at the end.

**GT-v6.9** (13 tags):

```
pixelalignment, fibrealignment, mppcalignment, tilealignment
pixelqualitylm, fibrequality, tilequality
eventstuffv1, detsetupv1
pixelmask, pixelefficiency
pixeltimecalibration, tiletimecalibration
```

**GT-v7.1** is the same except `eventstuffv2` replaces `eventstuffv1`.

Internal groups in `StartGT.pm`:

| Group | Calibrations | Notes |
|-------|--------------|-------|
| alignment | 4 alignment types | Needs prior `sim` (alignment ROOT) |
| quality | pixel/fibre/tile quality | Perfect IOV 1; mcideal fibre/tile use `*_ideal` tags |
| stuff | eventstuff, detsetup | Fixed ideal JSON under `db0/cdb2/ascii/` |
| mask | pixelmask | All pixels unmasked; uses `-u` test-CDB root |
| efficiency | pixelefficiency | mcideal tag: `pixelefficiency_ideal` |
| timecalib | pixel + tile time calibration | Zero shifts/offsets (ideal IOV 1) |

Individual group runners exist in `StartGT.pm` but are only invoked through `alltags` from the CLI today.

---

## Output: test-CDB

Under `{setup_basedir}/{mu3e_workdir}/run/{test_cdb_dir}/` (default `…/run/output/test-CDB/`):

```
test-CDB/
  payloads/     # calibration blobs
  tags/         # per-calibration tag JSON (IOV 1)
  globaltags/   # merged GT file, e.g. globaltags/mcidealv6.9
  runrecords/
  configs/
```

`alltags` writes payloads and tag files in sub-steps, then merges everything into one global tag file (existing tags on disk are preserved).

---

## Host overlay config

Create `config-<hostname>.cfg` (or pass `-H <name>` for `config-<name>.cfg`):

```
setup_basedir: "/path/to/prod/startGT"      # parent of mu3e-v6.9/, mu3e-v7.0pre0/, …
CDB_code_basedir: "/path/to/mu3eanca/db0/cdb2"
relink_script: "/path/to/mu3eanca/perl/relinkBinFiles"
cdb_dbconn: "/path/to/cdb/"                 # optional; for future DB use
```

`cdbRunPayloadWriter` is run from `CDB_code_basedir` and needs `lib/libCDB.so` there.

Examples in this directory: `config-moor.cfg`, `config-mu3edb0.cfg`.

---

## Typical workflow

```tcsh
# Check paths and whether alignment ROOT / tags exist:
./startGT -c GT-v6.9.cfg -H moor status
./startGT -c GT-v6.9.cfg -H moor -g mcideal status

# Full bootstrap for one flavour:
./startGT -c GT-v6.9.cfg -H moor setup
./startGT -c GT-v6.9.cfg -H moor sim
./startGT -c GT-v6.9.cfg -H moor -g mcideal alltags

# Same geometry, different installed-component year selection:
./startGT -c GT-v6.9.cfg -H moor -g mcrealistic alltags
./startGT -c GT-v6.9.cfg -H moor -g data alltags
```

After changing `db0/cdb2` (payload writers, ideal-input modes), rebuild:

```tcsh
cd /path/to/mu3eanca/db0/cdb2
make lib/libCDB.so bin/cdbRunPayloadWriter
```

---

## Related

- `db0/cdb2/bin/cdbRunPayloadWriter` — low-level payload and ideal-input writer
- `db0/cdb2/cdbInitGT.cc` — reference for tag naming conventions
- `../relval/README.md` — mu3e setup/run/compare workflow (shared `Setup` / `RelvalConfig`)
