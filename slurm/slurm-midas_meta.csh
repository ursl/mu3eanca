#! /bin/csh -f

#SBATCH --job-name=midas4758
#SBATCH -e %x.%j.err
#SBATCH -o %x.%j.out
#SBATCH --time=01:00:00
#SBATCH --mem=4G
#SBATCH -n 1

# Filled in by your submit wrapper (or set by hand):
# sbatch -p mu3e --export=RUNDIR,SETUP,RUN,TASK slurm-midas_meta.csh
if (! $?RUNDIR) setenv RUNDIR /data/experiment/mu3e/data/prompt/rereco/mu3e-260618-rereco
if (! $?SETUP)  setenv SETUP  /data/experiment/mu3e/data/prompt/rereco/setups/260618-rereco
if (! $?RUN)    setenv RUN    4758
if (! $?TASK)   setenv TASK   run${RUN}-midas_meta

echo "================================="
echo "====> SLURM midas_meta job  <===="
echo "================================="
date
hostname
limit coredumpsize 0

# -- basic environment
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

set MID_FILE = /data/experiment/mu3e/data/2025/raw/004/run0${RUN}.mid.lz4
set EXE      = $RUNDIR/mu3e/_build/modules/mu3eUtil/tools/midasMeta/mu3e_midas_meta
set SCRIPT   = $SETUP/common/scripts/run_midas_meta_file
set MARKER   = .markers/midas_meta-${TASK}.done

echo "--> input:  $MID_FILE"
echo "--> exe:    $EXE"
echo "--> marker: $MARKER"

perl "$SCRIPT" \
    --exe "$EXE" \
    --input-file "$MID_FILE" \
    --marker "$MARKER" \
    --log-prefix rereco

set st = $status
date
if ( $st != 0 ) then
    echo "run: FAILED with exit $st"
    exit $st
endif

echo "run: done, marker:"
ls -l $MARKER
echo "run: This is the end, my friend"
