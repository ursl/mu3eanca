#! /bin/bash -f

# --- paths (mu3edb0) ---
MU3E_RELVAL_BASEDIR="/mnt/data2/relval"
MU3E_DIRNAME1="mu3e-relval_mu3e-v6.7pre0_mcidealv6.5"
MU3E_DIRNAME0="mu3e-relval_mu3e-v6.6_mcidealv6.5"

SCENARIO="signal"
OBJ="sensors"

OUT_NAME="histocompare-${SCENARIO}-${OBJ}"
COMPARE_DIR="${MU3E_RELVAL_BASEDIR}/${MU3E_DIRNAME1}/run/output/compare/${SCENARIO}__${MU3E_DIRNAME1}__vs__${MU3E_DIRNAME0}"

DUMP1_HOST="${MU3E_RELVAL_BASEDIR}/${MU3E_DIRNAME1}/run/output/treedump-${SCENARIO}-${OBJ}.root"
DUMP0_HOST="${MU3E_RELVAL_BASEDIR}/${MU3E_DIRNAME0}/run/output/treedump-${SCENARIO}-${OBJ}.root"
# Paths inside the container (/relval is the mount of MU3E_RELVAL_BASEDIR)
DUMP1="/relval/${MU3E_DIRNAME1}/run/output/treedump-${SCENARIO}-${OBJ}.root"
DUMP0="/relval/${MU3E_DIRNAME0}/run/output/treedump-${SCENARIO}-${OBJ}.root"

# --- sanity (host paths) ---
ls -l "$DUMP0_HOST" "$DUMP1_HOST"
mkdir -p "$COMPARE_DIR"

HC_TMP=$(mktemp -d /tmp/relval-histocompare.XXXXXX)
HC_CNAME="relval-hc-manual-$$"
PODMAN_LOG="$COMPARE_DIR/${OUT_NAME}.podman.log"

podman rm -f "$HC_CNAME" 2>/dev/null || true

echo "[relval] container inputs: $DUMP1 (ref/new)  $DUMP0 (reference)"

podman create --name "$HC_CNAME" --userns=keep-id \
  -w /tmp \
  -e CLING_STANDARD_PCH=0 \
  -v "${MU3E_RELVAL_BASEDIR}:/relval:ro,Z" \
  --mount type=tmpfs,destination=/out,tmpfs-size=2G \
  docker.io/mu3e/histocompare \
  "$DUMP1" \
  "$DUMP0" \
  --treedump --threshold 0.60 --wasserstein 0.60 --accFailFraction 0.05 \
  --pdf -o "/out/${OUT_NAME}" \
  --skip=eventWeight --skip=farm_status \
  --skip=frameID --skip=runID --skip=mc_eventID --skip=mc_weight \
  --twoDimThreshold 1.0

podman start -a "$HC_CNAME" 2>&1 | tee "$PODMAN_LOG"
RC=${PIPESTATUS[0]}
echo "podman start rc=$RC"

podman cp "$HC_CNAME:/out/." "$HC_TMP/"
echo "=== extracted to $HC_TMP ==="
ls -la "$HC_TMP"

podman rm -f "$HC_CNAME"

# copy into compare dir (same as Snakemake)
cp -fv "$HC_TMP/${OUT_NAME}.pdf" "$COMPARE_DIR/" 2>/dev/null || echo "no pdf"
cp -fv "$HC_TMP/${OUT_NAME}.root" "$COMPARE_DIR/" 2>/dev/null || echo "no root"
cp -fv "$HC_TMP/${OUT_NAME}.log" "$COMPARE_DIR/" 2>/dev/null || echo "no log"

rm -rf "$HC_TMP"
echo "=== final compare dir ==="
ls -la "$COMPARE_DIR/${OUT_NAME}."*

