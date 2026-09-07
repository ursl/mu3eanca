#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o
#SBATCH --mem=8G

setenv JOB
setenv RUN
setenv SORTEDDIR
setenv SORTEDFILE
setenv ROOTFILE
setenv GT
setenv CDB
setenv ANLZR
setenv STORAGE1

echo "====================================================="
echo "====> SLURM RUN2026 prompt0 trirec wrapper       <===="
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

echo "--> Run trirec"
echo "pwd"
pwd
echo "ls -l"
ls -l
echo "ls -l $SORTEDDIR/$SORTEDFILE"
ls -l $SORTEDDIR/$SORTEDFILE
echo "ls -l mu3e_alignment.root"
ls -l mu3e_alignment.root

cp $SORTEDDIR/$SORTEDFILE .

if ( "$CDB" == "" ) then
    setenv CDB rest
endif

echo "../_build/mu3eTrirec/mu3eTrirec $ANLZR ./$SORTEDFILE --print-config=. --cdb.dbconn=$CDB --cdb.globalTag=$GT --output ./$ROOTFILE"
../_build/mu3eTrirec/mu3eTrirec $ANLZR ./$SORTEDFILE --print-config=. --cdb.dbconn=$CDB --cdb.globalTag=$GT --output ./$ROOTFILE

ls -l ./$ROOTFILE

date
ls -rtl
echo "slurm check size of rootfile produced"
echo "pwd"
pwd
echo "ls -l `pwd`"
ls -l `pwd`

echo "cp ./$ROOTFILE $STORAGE1/$RUN/$ROOTFILE"
cp ./$ROOTFILE $STORAGE1/$RUN/$ROOTFILE
echo "slurm check that rootfile was copied to $STORAGE1/$RUN"
ls -l $STORAGE1/$RUN/$ROOTFILE

date

# BATCH END

date
echo "run: This is the end, my friend"
