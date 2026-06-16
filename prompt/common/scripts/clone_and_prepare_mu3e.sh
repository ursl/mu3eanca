#!/usr/bin/env bash
# Clone or init-in-place, fetch, checkout tag, update submodules.
set -euo pipefail

REPO=""
TAG=""
WORK_BASEDIR=""
MU3E_DIR=""
MARKER=""

usage() {
    echo "Usage: $0 --repo REPO --tag TAG --work-basedir DIR --mu3e-dir DIR --marker FILE" >&2
    exit 2
}

while [ $# -gt 0 ]; do
    case "$1" in
        --repo=*) REPO="${1#*=}"; shift ;;
        --repo) REPO="$2"; shift 2 ;;
        --tag=*) TAG="${1#*=}"; shift ;;
        --tag) TAG="$2"; shift 2 ;;
        --work-basedir=*) WORK_BASEDIR="${1#*=}"; shift ;;
        --work-basedir) WORK_BASEDIR="$2"; shift 2 ;;
        --mu3e-dir=*) MU3E_DIR="${1#*=}"; shift ;;
        --mu3e-dir) MU3E_DIR="$2"; shift 2 ;;
        --marker=*) MARKER="${1#*=}"; shift ;;
        --marker) MARKER="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) echo "unknown option: $1" >&2; usage ;;
    esac
done

: "${REPO:?}" "${TAG:?}" "${WORK_BASEDIR:?}" "${MU3E_DIR:?}" "${MARKER:?}"

mkdir -p "$(dirname "$MARKER")"
mkdir -p "$WORK_BASEDIR"

if [ ! -d "$MU3E_DIR/.git" ]; then
    # workdir is MU3E_DIR; it may already contain .snakemake/.markers.
    if [ -z "$(ls -A "$MU3E_DIR" 2>/dev/null || true)" ]; then
        git clone "$REPO" "$MU3E_DIR"
    else
        git -C "$MU3E_DIR" init
        if ! git -C "$MU3E_DIR" remote get-url origin >/dev/null 2>&1; then
            git -C "$MU3E_DIR" remote add origin "$REPO"
        fi
    fi
fi

cd "$MU3E_DIR"
git fetch --force origin
git fetch --tags --force origin
git checkout "$TAG"
git submodule update --init --recursive

touch "$MARKER"
