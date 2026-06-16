#!/usr/bin/env bash
# Clone or init-in-place, fetch, checkout a tag/ref or branch HEAD,
# optionally merge extra commits (e.g. PR tips), update submodules.
set -euo pipefail

REPO=""
REF=""
BRANCH=""
WORK_BASEDIR=""
MU3E_DIR=""
MARKER=""
MERGES=()

usage() {
    echo "Usage: $0 --repo REPO --work-basedir DIR --mu3e-dir DIR --marker FILE \\" >&2
    echo "         (--ref REF | --branch BRANCH) [--merge COMMIT ...]" >&2
    echo "  --ref REF         Detached checkout of tag or other git ref." >&2
    echo "  --branch BRANCH   Track origin/BRANCH HEAD (reset --hard)." >&2
    echo "  --merge COMMIT    After checkout, git merge COMMIT (repeatable)." >&2
    exit 2
}

while [ $# -gt 0 ]; do
    case "$1" in
        --repo=*) REPO="${1#*=}"; shift ;;
        --repo) REPO="$2"; shift 2 ;;
        --ref=*) REF="${1#*=}"; shift ;;
        --ref) REF="$2"; shift 2 ;;
        --tag=*) REF="${1#*=}"; shift ;;
        --tag) REF="$2"; shift 2 ;;
        --branch=*) BRANCH="${1#*=}"; shift ;;
        --branch) BRANCH="$2"; shift 2 ;;
        --merge=*) MERGES+=("${1#*=}"); shift ;;
        --merge) MERGES+=("$2"); shift 2 ;;
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

: "${REPO:?}" "${WORK_BASEDIR:?}" "${MU3E_DIR:?}" "${MARKER:?}"
if [ -z "$REF" ] && [ -z "$BRANCH" ]; then
    echo "ERROR: specify --ref or --branch" >&2
    usage
fi
if [ -n "$REF" ] && [ -n "$BRANCH" ]; then
    echo "ERROR: use only one of --ref or --branch" >&2
    usage
fi

mkdir -p "$(dirname "$MARKER")"
mkdir -p "$WORK_BASEDIR"

if [ ! -d "$MU3E_DIR/.git" ]; then
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

if [ -n "$BRANCH" ]; then
    if ! git fetch --force origin "$BRANCH"; then
        echo "ERROR: failed to fetch origin/$BRANCH" >&2
        exit 1
    fi
    if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
        git checkout "$BRANCH"
    else
        git checkout -B "$BRANCH" "origin/$BRANCH"
    fi
    git reset --hard "origin/$BRANCH"
    echo "Checked out branch $BRANCH at $(git rev-parse --short HEAD) (origin/$BRANCH)"
else
    git checkout "$REF"
    echo "Checked out ref $REF at $(git rev-parse --short HEAD)"
fi

ensure_commit() {
    local hash="$1"
    if git cat-file -e "${hash}^{commit}" 2>/dev/null; then
        return 0
    fi
    echo "Commit $hash not found locally; fetching origin again..."
    git fetch --force origin
    git fetch --tags --force origin
    if git cat-file -e "${hash}^{commit}" 2>/dev/null; then
        return 0
    fi
    echo "ERROR: commit $hash not available after fetch." >&2
    echo "       Fetch the PR branch into this clone first, e.g.:" >&2
    echo "         git fetch origin refs/pull-requests/49/from:pr-49" >&2
    exit 1
}

for hash in "${MERGES[@]}"; do
    ensure_commit "$hash"
    echo "Merging $hash ..."
    if ! git merge --no-edit -m "prompt checkout: merge $hash for testing" "$hash"; then
        echo "ERROR: git merge $hash failed (conflicts?). Resolve manually in $MU3E_DIR" >&2
        exit 1
    fi
    echo "After merge $hash: $(git rev-parse --short HEAD)"
done

git submodule update --init --recursive

touch "$MARKER"
