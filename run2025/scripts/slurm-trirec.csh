#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o

setenv JOB
setenv RUN
setenv DATADIR
setenv SORTEDDIR
setenv SORTEDFILE
setenv ROOTFILE
setenv GT
setenv ANLZR
setenv STORAGE1

# -- set up env required for running
#does not work: source /psi/home/langenegger/mu3e/setup.csh
#

echo "===================================================="
echo "====> SLURM RUN2025 processRuns/trirec wrapper <===="
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
cd mu3e/run

# ----------------------------------------------------------------------
# -- Run sort and trirec
# ----------------------------------------------------------------------
echo "--> Run sort and trirec"
echo "pwd"
pwd
echo "ls -l"
ls -l
echo "ls -l $SORTEDDIR/$SORTEDFILE"
ls -l $SORTEDDIR/$SORTEDFILE
echo "ls -l ../mu3eTrirec/trirec.conf"
ls -l ../mu3eTrirec/trirec.conf

cp $SORTEDDIR/$SORTEDFILE .

# echo "../_build/mu3eTrirec/mu3eTrirec $ANLZR  ./$SORTEDFILE --conf ../mu3eTrirec/trirec.conf --output ./$ROOTFILE"
# ../_build/mu3eTrirec/mu3eTrirec $ANLZR ./$SORTEDFILE --conf ../mu3eTrirec/trirec.conf --output ./$ROOTFILE
echo "../_build/mu3eTrirec/mu3eTrirec  ./$SORTEDFILE --trirec.twolayer.mode=1 --cdb.dbconn=rest --cdb.globalTag=$GT --conf ../mu3eTrirec/trirec.conf --output ./$ROOTFILE"
../_build/mu3eTrirec/mu3eTrirec $ANLZR ./$SORTEDFILE --trirec.twolayer.mode=1 --cdb.dbconn=rest --cdb.globalTag=$GT --conf ../mu3eTrirec/trirec.conf --output ./$ROOTFILE

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
setenv BLA  `ls -l $STORAGE1/$RUN/$ROOTFILE`
echo "slurm check that rootfile was copied ->$BLA<-"
ls -l $STORAGE1/$RUN/$ROOTFILE

echo "cp ./$SORTEDFILE $STORAGE1/$RUN/$SORTEDFILE"
cp ./$SORTEDFILE $STORAGE1/$RUN/$SORTEDFILE
setenv BLA  `ls -l $STORAGE1/$RUN/$SORTEDFILE`
echo "slurm check that sortedfile was copied ->$BLA<-"
ls -l $STORAGE1/$RUN/$SORTEDFILE


date

# BATCH END

date
echo "run: This is the end, my friend"
