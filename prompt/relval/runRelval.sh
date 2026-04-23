#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 [--setup-name NAME] [snakemake args...] --config mu3e_tag=<tag> [other config overrides]"
  echo "Example: $0 --setup-name relval-v65 -j4 -p --config mu3e_tag=v6.5 cdb_GT=mcidealv6.5"
  exit 1
fi

SETUP_NAME=""
declare -a FORWARDED_ARGS=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --setup-name)
      if [[ $# -lt 2 ]]; then
        echo "ERROR: --setup-name requires a value"
        exit 1
      fi
      SETUP_NAME="$2"
      shift 2
      ;;
    *)
      FORWARDED_ARGS+=("$1")
      shift
      ;;
  esac
done

MU3E_TAG=""
RAW_CDB_GT=""
RAW_CDB_DBCONN=""
for arg in "${FORWARDED_ARGS[@]}"; do
  if [[ "$arg" == mu3e_tag=* ]]; then
    MU3E_TAG="${arg#mu3e_tag=}"
  elif [[ "$arg" == cdb_GT=* ]]; then
    RAW_CDB_GT="${arg#cdb_GT=}"
  elif [[ "$arg" == cdb_gt=* ]]; then
    RAW_CDB_GT="${arg#cdb_gt=}"
  elif [[ "$arg" == cdb_dbconn=* ]]; then
    RAW_CDB_DBCONN="${arg#cdb_dbconn=}"
  elif [[ "$arg" == cdb_db_conn=* ]]; then
    RAW_CDB_DBCONN="${arg#cdb_db_conn=}"
  fi
done

if [[ -z "$MU3E_TAG" ]]; then
  echo "ERROR: missing mu3e_tag in arguments."
  echo "Please pass it as Snakemake config, e.g. --config mu3e_tag=v6.5"
  exit 1
fi

cd "$SCRIPT_DIR"

# Derive workdir the same way as Snakefile:
#   <mu3e_relval_basedir>/<mu3e_dir>-<mu3e_tag_with_slashes_replaced>
BASE_DIR="$(sed -n 's/^mu3e_relval_basedir:[[:space:]]*"\(.*\)"/\1/p' config.yaml | head -n1)"
MU3E_DIR_BASE="$(sed -n 's/^mu3e_dir:[[:space:]]*"\(.*\)"/\1/p' config.yaml | head -n1)"

if [[ -z "$RAW_CDB_GT" ]]; then
  RAW_CDB_GT="$(sed -n 's/^cdb_GT:[[:space:]]*"\(.*\)"/\1/p' config.yaml | head -n1)"
fi
if [[ -z "$RAW_CDB_DBCONN" ]]; then
  RAW_CDB_DBCONN="$(sed -n 's/^cdb_dbconn:[[:space:]]*"\(.*\)"/\1/p' config.yaml | head -n1)"
fi

if [[ -z "$SETUP_NAME" ]]; then
  SETUP_NAME="relval_${MU3E_DIR_BASE}-${MU3E_TAG}_${RAW_CDB_GT}"
fi

SETUP_SAFE="${SETUP_NAME//\//_}"
SETUP_SAFE="${SETUP_SAFE// /_}"

SETUP_DIR="${BASE_DIR}/setups/${SETUP_SAFE}"
mkdir -p "$SETUP_DIR"

cp "$SCRIPT_DIR/Snakefile" "$SETUP_DIR/Snakefile"
cp "$SCRIPT_DIR/config.yaml" "$SETUP_DIR/config.yaml"

# Freeze key setup identity in the generated config for reproducibility and isolation.
python3 - "$SETUP_DIR/config.yaml" "${MU3E_DIR_BASE}-${SETUP_SAFE}" "$MU3E_TAG" "$RAW_CDB_GT" "$RAW_CDB_DBCONN" <<'PY'
import sys
from pathlib import Path

cfg = Path(sys.argv[1])
mu3e_workdir_name = sys.argv[2]
mu3e_tag = sys.argv[3]
cdb_gt = sys.argv[4]
cdb_dbconn = sys.argv[5]

lines = cfg.read_text().splitlines()
out = []
seen_workdir_name = False
seen_checkout_tag = False
for line in lines:
    if line.startswith("mu3e_workdir_name:"):
        out.append(f'mu3e_workdir_name: "{mu3e_workdir_name}"')
        seen_workdir_name = True
    elif line.startswith("mu3e_checkout_tag:"):
        out.append(f'mu3e_checkout_tag: "{mu3e_tag}"')
        seen_checkout_tag = True
    elif line.startswith("mu3e_tag:"):
        out.append(f'mu3e_tag: "{mu3e_tag}"')
    elif line.startswith("cdb_GT:"):
        out.append(f'cdb_GT: "{cdb_gt}"')
    elif line.startswith("cdb_dbconn:"):
        out.append(f'cdb_dbconn: "{cdb_dbconn}"')
    else:
        out.append(line)

if not seen_workdir_name:
    out.append(f'mu3e_workdir_name: "{mu3e_workdir_name}"')
if not seen_checkout_tag:
    out.append(f'mu3e_checkout_tag: "{mu3e_tag}"')

cfg.write_text("\n".join(out) + "\n")
PY

WORKDIR="${BASE_DIR}/${MU3E_DIR_BASE}-${SETUP_SAFE}"

# Run workflow from the generated setup directory.
cd "$SETUP_DIR"
snakemake "${FORWARDED_ARGS[@]}"

# Export status metadata into the corresponding workdir.
STATUS_DIR="${WORKDIR}/status"
mkdir -p "$STATUS_DIR"

snakemake --summary "${FORWARDED_ARGS[@]}" > "$STATUS_DIR/summary.tsv"
snakemake --detailed-summary "${FORWARDED_ARGS[@]}" > "$STATUS_DIR/detailed-summary.tsv"

echo "Setup directory: $SETUP_DIR"
echo "Status exported to: $STATUS_DIR"
