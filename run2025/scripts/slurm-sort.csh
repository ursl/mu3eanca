#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o
#SBATCH --mem=8G

setenv JOB
setenv RUN
setenv DATADIR
setenv MIDASFILE
setenv SORTEDFILE
setenv ROOTFILE
setenv GT
setenv ANLZR
setenv STORAGE1

# -- set up env required for running
#does not work: source /psi/home/langenegger/mu3e/setup.csh
#

echo "===================================================="
echo "====> SLURM RUN2025 processRuns/sort wrapper <===="
echo "===================================================="
date

# ----------------------------------------------------------------------
# -- The Basics
# ----------------------------------------------------------------------
echo "--> Environment"
hostname
uname -a
limit coredumpsize 0
printenv

pwd
echo "--> End of env testing"

# BATCH START


# ----------------------------------------------------------------------
# -- setup runtime directory
# ----------------------------------------------------------------------
echo "--> Extract tar file"
date
tar zxf ./$JOB.tar.gz
cd mu3e/run

# ----------------------------------------------------------------------
# -- Run sort
# ----------------------------------------------------------------------
echo "--> Run sort"
echo "pwd"
pwd
echo "ls -l"
ls -l
echo "ls -l $DATADIR/$MIDASFILE"
ls -l $DATADIR/$MIDASFILE
echo "ls -l mu3e_alignment.root"
ls -l mu3e_alignment.root

echo "../_build/mu3eSim/sort/mu3eSort $ANLZR  $DATADIR/$MIDASFILE --output ./$SORTEDFILE"
../_build/mu3eSim/sort/mu3eSort $ANLZR  $DATADIR/$MIDASFILE --output ./$SORTEDFILE

ls -l ./$SORTEDFILE

date
ls -rtl
echo "slurm check size of rootfile produced"
echo "pwd"
pwd
echo "ls -l `pwd`"
ls -l `pwd`

echo "cp ./$SORTEDFILE $STORAGE1/$RUN/$SORTEDFILE"
cp ./$SORTEDFILE $STORAGE1/$RUN/$SORTEDFILE
setenv BLA  `ls -l $STORAGE1/$RUN/$SORTEDFILE`
echo "slurm check that sortedfile was copied ->$BLA<-"
ls -l $STORAGE1/$RUN/$SORTEDFILE

date

# BATCH END

date
echo "run: This is the end, my friend"
