#! /bin/csh -f

#SBATCH --job-name=midas_meta
#SBATCH -e
#SBATCH -o
#SBATCH --time=01:00:00
#SBATCH -n 1
# memory: set via sbatch --mem from run_midas_meta_submit (config slurm_mem)

# Required env (set by run_midas_meta_submit before sbatch):
#   RUNDIR, EXE, MID_FILE, MARKER, SCRIPT
# Optional: SETUP, TASK, RUN, LOG_PREFIX

echo "================================="
echo "====> SLURM midas_meta job  <===="
echo "================================="
date
hostname
limit coredumpsize 0

if (! $?LOG_PREFIX) setenv LOG_PREFIX rereco
if (! $?RUNDIR) then
    echo "[$LOG_PREFIX] ERROR: RUNDIR not set" >&2
    exit 1
endif
if (! $?EXE) then
    echo "[$LOG_PREFIX] ERROR: EXE not set" >&2
    exit 1
endif
if (! $?MID_FILE) then
    echo "[$LOG_PREFIX] ERROR: MID_FILE not set" >&2
    exit 1
endif
if (! $?MARKER) then
    echo "[$LOG_PREFIX] ERROR: MARKER not set" >&2
    exit 1
endif
if (! $?SCRIPT) then
    echo "[$LOG_PREFIX] ERROR: SCRIPT not set" >&2
    exit 1
endif

# -- basic environment (merlin batch nodes)
setenv SW "/data/experiment/mu3e/code/software"

setenv Boost_DIR ${SW}
setenv Eigen3_DIR ${SW}
setenv LZ4_ROOT ${SW}

setenv PATH ${SW}/bin:${PATH}
setenv LD_LIBRARY_PATH ${SW}/lib:${LD_LIBRARY_PATH}

setenv Geant4_ROOT ${SW}
setenv Geant4_PREFIX `geant4-config --prefix`
setenv ROOT_ROOT ${SW}
setenv ROOTSYS ${SW}

cd $RUNDIR || exit 1
pwd

echo "--> input:  $MID_FILE"
echo "--> exe:    $EXE"
echo "--> marker: $MARKER"

perl "$SCRIPT" \
    --exe "$EXE" \
    --input-file "$MID_FILE" \
    --marker "$MARKER" \
    --log-prefix "$LOG_PREFIX"

set st = $status
date
if ( $st != 0 ) then
    echo "[$LOG_PREFIX] FAILED with exit $st"
    exit $st
endif

echo "[$LOG_PREFIX] done, marker:"
ls -l $MARKER
echo "[$LOG_PREFIX] This is the end, my friend"
