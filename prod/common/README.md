# Shared Snakemake pieces for `prompt/relval`, `prompt/rereco`, …

## Layout

```
common/
  RerecoRuns                    run-list / task template expansion (rereco)
  load_rereco_tasks             JSON stdin → JSON task list (Snakemake helper)
  mu3e_prepare.smk              clone / build / relink rules (mu3e checkout)
  mu3e_util_bootstrap.smk         standalone mu3eUtil build (legacy; not used by rereco)
  mu3e_trirec.smk               shared run_mu3e_trirec rule
  midas_meta.smk                run mu3e_midas_meta on *.mid.lz4 inputs
  scripts/
    clone_and_prepare_mu3e      git checkout implementation
    run_midas_meta_dir          midas meta batch driver
    run_midas_meta_file         midas meta single-file driver
```

## Usage in a workflow Snakefile

1. Resolve `prompt/common` (local `common/` copy in setup dir, or sibling `../common`).
2. Set variables expected by `mu3e_prepare.smk`.
3. `include:` the `.smk` file.

```python
_PROMPT_COMMON = _base / "common"
if not _PROMPT_COMMON.is_dir():
    _PROMPT_COMMON = _base.parent / "common"

MU3E_WORK_BASEDIR = ...          # relval or rereco base dir
MU3E_PREP_LOG_PREFIX = "relval"  # log tag for relink rule
CLONE_MU3E_SCRIPT = str(_PROMPT_COMMON / "scripts" / "clone_and_prepare_mu3e")
CLONE_MU3E_INPUTS = [...]        # one prerequisite path
MARKER_DIR = "markers"           # or from config["marker_dir"]

include: str(_PROMPT_COMMON / "mu3e_prepare.smk")
```

`initRereco` / `runRelval` copy this tree into each setup directory as `common/`.

## `mu3e_trirec.smk`

Include after workflow-specific helpers and **after** `CDB_DBCONN` / `CDB_GT` are set.

| Callback | Purpose |
|----------|---------|
| `trirec_rule_inputs(wc)` | Snakemake input file list |
| `trirec_sort_input(wc)` | Sort ROOT path for `mu3eTrirec` |
| `trirec_output_rel(wc)` | Output path relative to `run/` |
| `trirec_conf_for(wc)` / `trirec_conf_fallback_for(wc)` | Conf files under `run/` |
| `trirec_run_id_for(wc)` | `--run` argument |
| `trirec_item_label(wc)` | Id string for log lines |
| `TRIREC_OUTPUT_TPL` / `TRIREC_LOG_TPL` | Output and log path templates |

Relval uses wildcard `{scenario}`; rereco uses `{job}` — each workflow keeps its own template strings and maps them in the callbacks.

## Required symbols (must exist before `include` of `mu3e_prepare.smk`)

| Name | Meaning |
|------|---------|
| `MU3E_REPO`, `MU3E_CHECKOUT_REF`, `MU3E_CHECKOUT_BRANCH`, `MU3E_CHECKOUT_MERGES`, `MU3E_DIR` | MU3E checkout |
| `MU3E_WORK_BASEDIR` | Snakemake workdir / setup root (rereco: `{basedir}/mu3e-{setup}`; relval: **same path as** `MU3E_DIR`) |
| `MARKER_DIR` | Completion marker directory under workdir (default: `markers`) |
| `MIDAS_META_PREREQS` | Inputs for `run_midas_meta` (rereco: `[markers/build_mu3e.done]`) |
| `MIDAS_META_EXE` | Path to `mu3e_midas_meta` under `mu3e/_build/` |
| `MAKE_JOBS`, `RELINK_SCRIPT` | build + relink |
| `CLONE_MU3E_INPUTS` | Single-item list: bootstrap marker (relval) or local deps marker (rereco) |
| `CLONE_MU3E_SCRIPT` | Path to `clone_and_prepare_mu3e` |
| `MU3E_PREP_LOG_PREFIX` | Short name in log lines |

`clone_and_prepare_mu3e` treats a directory that only contains Snakemake metadata (`.snakemake`, `markers`, `logs`, `status`; legacy `.markers` also accepted) as unset and runs `git init` + fetch/checkout there. That case arises in **relval**, where the workdir is the MU3E tree itself.
