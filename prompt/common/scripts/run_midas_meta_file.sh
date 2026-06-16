#!/usr/bin/env bash
# Run mu3e_midas_meta on a single *.mid.lz4 file.
set -euo pipefail

EXE=""
INPUT_FILE=""
MARKER=""
LOG_PREFIX="rereco"

usage() {
    echo "Usage: $0 --exe PATH --input-file FILE --marker FILE [--log-prefix NAME]" >&2
    exit 2
}

while [ $# -gt 0 ]; do
    case "$1" in
        --exe=*) EXE="${1#*=}"; shift ;;
        --exe) EXE="$2"; shift 2 ;;
        --input-file=*) INPUT_FILE="${1#*=}"; shift ;;
        --input-file) INPUT_FILE="$2"; shift 2 ;;
        --marker=*) MARKER="${1#*=}"; shift ;;
        --marker) MARKER="$2"; shift 2 ;;
        --log-prefix=*) LOG_PREFIX="${1#*=}"; shift ;;
        --log-prefix) LOG_PREFIX="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) echo "unknown option: $1" >&2; usage ;;
    esac
done

: "${EXE:?}" "${INPUT_FILE:?}" "${MARKER:?}"

if [ ! -x "$EXE" ]; then
    echo "[$LOG_PREFIX] ERROR: mu3e_midas_meta not executable: $EXE" >&2
    exit 1
fi
if [ ! -f "$INPUT_FILE" ]; then
    echo "[$LOG_PREFIX] ERROR: input file missing: $INPUT_FILE" >&2
    exit 1
fi

mkdir -p "$(dirname "$MARKER")"
echo "[$LOG_PREFIX] midas_meta: $INPUT_FILE"
"$EXE" "$INPUT_FILE"
touch "$MARKER"
