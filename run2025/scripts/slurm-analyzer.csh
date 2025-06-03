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

echo "====================================================="
echo "====> SLURM RUN2025 processRuns wrapper         <===="
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
echo "--> check visibility of /data/experiment/mu3e/code/offline"
ls -l /data/experiment/mu3e/code/offline

echo "--> End of env testing"

# BATCH START


# ----------------------------------------------------------------------
# -- setup runtime directory
# ----------------------------------------------------------------------
echo "--> Extract tar file"
date
tar zxf ./$JOB.tar.gz
cd minalyzer

# ----------------------------------------------------------------------
# -- Run analyzer
# ----------------------------------------------------------------------
echo "--> Run analyzer"
echo "pwd"
pwd
echo "ls -l"
ls -l

echo "_build/analyzer/minalyzer $ANLZR $PCRDATADIR/$MIDASFILE -- offline cdb.dbconn=rest cdb.gt=datav6.1=2025CosmicsVtxOnly"
_build/analyzer/minalyzer $ANLZR $PCRDATADIR/$MIDASFILE  -- offline cdb.dbconn=rest cdb.gt=datav6.1=2025CosmicsVtxOnly
date
ls -rtl
echo "slurm check size of rootfile produced"
echo "pwd"
pwd
echo "ls -l `pwd`/root_output_files"
ls -l `pwd`/root_output_files

echo "cp -r ./root_output_files $STORAGE1/$RUN/root_output_files"
cp -r root_output_files $STORAGE1/$RUN/root_output_files_$JOB

echo "slurm check that rootfile was copied to $STORAGE1/$RUN"
ls -l $STORAGE1/$RUN

date

# BATCH END

date
echo "run: This is the end, my friend"
