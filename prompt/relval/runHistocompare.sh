#! /bin/bash -f

# --- paths (mu3edb0) ---
#MU3E_RELVAL_BASEDIR="/mnt/data2/relval"
MU3E_RELVAL_BASEDIR="/Users/ursl/data/mu3e/relval"
MU3E_DIRNAME1="mu3e-relval_mu3e-v6.7pre0_mcidealv6.5"
MU3E_DIRNAME0="mu3e-relval_mu3e-v6.6_mcidealv6.5"

SCENARIO="signal"
OBJ="sensors"

OUT_NAME="histocompare-${SCENARIO}-${OBJ}"
COMPARE_DIR="${MU3E_RELVAL_BASEDIR}/${MU3E_DIRNAME1}/run/output/compare/${SCENARIO}__${MU3E_DIRNAME1}__vs__${MU3E_DIRNAME0}"
echo "COMPARE_DIR: $COMPARE_DIR"

DUMP1_HOST="${MU3E_RELVAL_BASEDIR}/${MU3E_DIRNAME1}/run/output/treedump-${SCENARIO}-${OBJ}.root"
DUMP0_HOST="${MU3E_RELVAL_BASEDIR}/${MU3E_DIRNAME0}/run/output/treedump-${SCENARIO}-${OBJ}.root"
# Paths inside the container (/relval is the mount of MU3E_RELVAL_BASEDIR)
DUMP1="/relval/${MU3E_DIRNAME1}/run/output/treedump-${SCENARIO}-${OBJ}.root"
DUMP0="/relval/${MU3E_DIRNAME0}/run/output/treedump-${SCENARIO}-${OBJ}.root"

DOCKER="${DOCKER:-docker}"
IMAGE="${HISTOCOMPARE_IMAGE:-docker.io/mu3e/histocompare}"

# --- sanity (host paths) ---
ls -l "$DUMP0_HOST" "$DUMP1_HOST"
mkdir -p "$COMPARE_DIR"

HC_TMP=$(mktemp -d /tmp/relval-histocompare.XXXXXX)
chmod u+rwx "$HC_TMP"
DOCKER_LOG="$COMPARE_DIR/${OUT_NAME}.docker.log"

echo "[relval] container inputs: $DUMP1 (new)  $DUMP0 (reference)"

set -o pipefail
$DOCKER run --rm \
  --user "$(id -u):$(id -g)" \
  -w /tmp \
  -e CLING_STANDARD_PCH=0 \
  -v "${MU3E_RELVAL_BASEDIR}:/relval:ro" \
  -v "$HC_TMP:/workdir" \
  "$IMAGE" \
  "$DUMP1" \
  "$DUMP0" \
  --treedump --threshold 0.60 --wasserstein 0.60 --accFailFraction 0.05 \
  --pdf -o "/workdir/${OUT_NAME}" \
  --skip=eventWeight --skip=farm_status \
  --skip=frameID --skip=runID --skip=mc_eventID --skip=mc_weight \
  --twoDimThreshold 1.0 \
  2>&1 | tee "$DOCKER_LOG"
RC=${PIPESTATUS[0]}
set +o pipefail
echo "docker run rc=$RC"

echo "=== workdir $HC_TMP ==="
ls -la "$HC_TMP"

# copy into compare dir (same as Snakemake)
cp -fv "$HC_TMP/${OUT_NAME}.pdf" "$COMPARE_DIR/" 2>/dev/null || echo "no pdf"
cp -fv "$HC_TMP/${OUT_NAME}.root" "$COMPARE_DIR/" 2>/dev/null || echo "no root"
cp -fv "$HC_TMP/${OUT_NAME}.log" "$COMPARE_DIR/" 2>/dev/null || echo "no log"

rm -rf "$HC_TMP"
echo "=== final compare dir ==="
ls -la "$COMPARE_DIR/${OUT_NAME}."*
