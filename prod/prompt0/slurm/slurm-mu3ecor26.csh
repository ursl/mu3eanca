#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o
#SBATCH --mem=8G

setenv JOB
setenv RUN
setenv MU3EANA
setenv EXE
setenv SORTEDFILE
setenv RESNAME
setenv STORAGE1

echo "====================================================="
echo "====> SLURM RUN2026 prompt0 mu3ecor26 wrapper    <===="
echo "====================================================="
date

echo "--> Environment"
hostname
uname -a
limit coredumpsize 0
printenv

pwd
echo "--> End of env testing"

# BATCH START

# The binary lives in the mu3eana checkout and writes results/ under cwd.
echo "--> Run mu3eCor26"
cd $MU3EANA
mkdir -p results
echo "pwd"
pwd
echo "ls -l $SORTEDFILE"
ls -l $SORTEDFILE

echo "$EXE $SORTEDFILE $RESNAME -1"
$EXE $SORTEDFILE $RESNAME -1
if ( $status != 0 ) then
    echo "mu3eCor26 failed"
    exit 1
endif

set PDF = results/$RESNAME/$RESNAME.pdf
ls -l $PDF

echo "cp $PDF $STORAGE1/$RUN/$RESNAME.pdf"
cp $PDF $STORAGE1/$RUN/$RESNAME.pdf
echo "slurm check that pdf was copied to $STORAGE1/$RUN"
ls -l $STORAGE1/$RUN/$RESNAME.pdf

date

# BATCH END

date
echo "run: This is the end, my friend"
