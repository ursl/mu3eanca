#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o

setenv JOB
setenv RUN
setenv PCRDATADIR
setenv MIDASFILE
setenv ROOTFILE
setenv ANLZR
setenv STORAGE1

# -- set up env required for running
#does not work: source /psi/home/langenegger/mu3e/setup.csh
#

echo "===================================================="
echo "====> SLURM mdc2023 proCalRec/analyzer wrapper <===="
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
echo "--> check visibility of /psi/home/langenegger/data/mdc2023"
ls -l /psi/home/langenegger/data/mdc2023

echo "--> End of env testing"

# BATCH START


# ----------------------------------------------------------------------
# -- setup runtime directory
# ----------------------------------------------------------------------
echo "--> Extract tar file"
date
tar zxf ./$JOB.tar.gz
cd analyzer/_run

# ----------------------------------------------------------------------
# -- Run analyzer
# ----------------------------------------------------------------------
echo "--> Run analyzer"
echo "pwd"
pwd
echo "ls -l"
ls -l

echo "../_build/analyzer/mdcanalyzer $ANLZR $PCRDATADIR/$MIDASFILE"
../_build/analyzer/mdcanalyzer $ANLZR $PCRDATADIR/$MIDASFILE
date
ls -rtl
echo "slurm check size of rootfile produced"
echo "pwd"
pwd
echo "ls -l `pwd`/root_output_files"
ls -l `pwd`/root_output_files

echo "cp ./output*$RUN.root $STORAGE1/$RUN/$ROOTFILE"
cp root_output_files/output$RUN.root $STORAGE1/$RUN/$ROOTFILE
setenv BLA  `ls -l $STORAGE1/$RUN/$ROOTFILE`
echo "slurm check that rootfile was copied ->$BLA<-"
ls -l $STORAGE1/$RUN/$ROOTFILE

date

# BATCH END

date
echo "run: This is the end, my friend"
