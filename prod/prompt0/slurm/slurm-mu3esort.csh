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
setenv GT
setenv ANLZR
setenv STORAGE1

# -- set up env required for running
echo "====================================================="
echo "====> SLURM RUN2026 prompt0 sort wrapper         <===="
echo "====================================================="
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

echo "../_build/mu3eSim/sort/mu3eSort $ANLZR $DATADIR/$MIDASFILE --output ./$SORTEDFILE"
../_build/mu3eSim/sort/mu3eSort $ANLZR $DATADIR/$MIDASFILE --output ./$SORTEDFILE

ls -l ./$SORTEDFILE

date
ls -rtl
echo "slurm check size of sortedfile produced"
echo "pwd"
pwd
echo "ls -l `pwd`"
ls -l `pwd`

echo "cp ./$SORTEDFILE $STORAGE1/$RUN/$SORTEDFILE"
cp ./$SORTEDFILE $STORAGE1/$RUN/$SORTEDFILE
echo "slurm check that sortedfile was copied to $STORAGE1/$RUN"
ls -l $STORAGE1/$RUN/$SORTEDFILE

date

# BATCH END

date
echo "run: This is the end, my friend"
