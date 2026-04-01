#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REST_DIR="${SCRIPT_DIR}"
DB1_DIR="$(cd "${REST_DIR}/.." && pwd)"
RESTJSON_DIR="${DB1_DIR}/restJSON"

echo "[sync-restjson] rest master:    ${REST_DIR}"
echo "[sync-restjson] restJSON target: ${RESTJSON_DIR}"

if [[ ! -d "${RESTJSON_DIR}" ]]; then
  echo "[sync-restjson] ERROR: ${RESTJSON_DIR} does not exist" >&2
  exit 1
fi

cp "${REST_DIR}/public/cdb.html" "${RESTJSON_DIR}/public/cdb.html"
cp "${REST_DIR}/routes/cdbjson.mjs" "${RESTJSON_DIR}/routes/cdb.mjs"

python3 "${REST_DIR}/tools/restjson_overlay.py"

echo "[sync-restjson] done."
echo "[sync-restjson] optional check:"
echo "  diff -u \"${REST_DIR}/public/cdb.html\" \"${RESTJSON_DIR}/public/cdb.html\" || true"
