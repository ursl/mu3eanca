#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o
#SBATCH --mem=4G

setenv JOB
setenv RUN
setenv TRIRECDIR
setenv TRIRECFILE
setenv ROOTFILE
setenv GT
setenv CDB
setenv ANLZR
setenv STORAGE1

echo "====================================================="
echo "====> SLURM RUN2026 prompt0 vertex wrapper       <===="
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

echo "--> Extract tar file"
date
tar zxf ./$JOB.tar.gz
cd mu3e/run

echo "--> Run mu3eVertexFit"
echo "pwd"
pwd
echo "ls -l $TRIRECDIR/$TRIRECFILE"
ls -l $TRIRECDIR/$TRIRECFILE

cp $TRIRECDIR/$TRIRECFILE .

if ( "$CDB" == "" ) then
    setenv CDB rest
endif

echo "../_build/mu3eVertex/mu3eVertexFit ./$TRIRECFILE $ANLZR --cdb.dbconn=$CDB --cdb.globalTag=$GT --output ./$ROOTFILE"
../_build/mu3eVertex/mu3eVertexFit ./$TRIRECFILE $ANLZR --cdb.dbconn=$CDB --cdb.globalTag=$GT --output ./$ROOTFILE
if ( $status != 0 ) then
    echo "mu3eVertexFit failed"
    exit 1
endif

ls -l ./$ROOTFILE

date
echo "cp ./$ROOTFILE $STORAGE1/$RUN/$ROOTFILE"
cp ./$ROOTFILE $STORAGE1/$RUN/$ROOTFILE
echo "slurm check that rootfile was copied to $STORAGE1/$RUN"
ls -l $STORAGE1/$RUN/$ROOTFILE

date

# BATCH END

date
echo "run: This is the end, my friend"
