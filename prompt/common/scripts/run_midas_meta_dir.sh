#!/usr/bin/env bash
# Run mu3e_midas_meta on every *.mid.lz4 file in a directory.
set -euo pipefail

EXE=""
INPUT_DIR=""
MARKER=""
LOG_PREFIX="rereco"

usage() {
    echo "Usage: $0 --exe PATH --input-dir DIR --marker FILE [--log-prefix NAME]" >&2
    exit 2
}

while [ $# -gt 0 ]; do
    case "$1" in
        --exe=*) EXE="${1#*=}"; shift ;;
        --exe) EXE="$2"; shift 2 ;;
        --input-dir=*) INPUT_DIR="${1#*=}"; shift ;;
        --input-dir) INPUT_DIR="$2"; shift 2 ;;
        --marker=*) MARKER="${1#*=}"; shift ;;
        --marker) MARKER="$2"; shift 2 ;;
        --log-prefix=*) LOG_PREFIX="${1#*=}"; shift ;;
        --log-prefix) LOG_PREFIX="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) echo "unknown option: $1" >&2; usage ;;
    esac
done

: "${EXE:?}" "${INPUT_DIR:?}" "${MARKER:?}"

if [ ! -x "$EXE" ]; then
    echo "[$LOG_PREFIX] ERROR: mu3e_midas_meta not executable: $EXE" >&2
    exit 1
fi
if [ ! -d "$INPUT_DIR" ]; then
    echo "[$LOG_PREFIX] ERROR: input directory missing: $INPUT_DIR" >&2
    exit 1
fi

shopt -s nullglob
files=("$INPUT_DIR"/*.mid.lz4)
if [ ${#files[@]} -eq 0 ]; then
    echo "[$LOG_PREFIX] ERROR: no *.mid.lz4 files in $INPUT_DIR" >&2
    exit 1
fi

mkdir -p "$(dirname "$MARKER")"
for f in "${files[@]}"; do
    echo "[$LOG_PREFIX] midas_meta: $f"
    "$EXE" "$f"
done

touch "$MARKER"
