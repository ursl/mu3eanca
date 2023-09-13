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

echo "=================================================="
echo "====> SLURM mdc2023 proCalRec/trirec wrapper <===="
echo "=================================================="
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
cd mu3e/run

# ----------------------------------------------------------------------
# -- Run trirec
# ----------------------------------------------------------------------
echo "--> Run trirec"
echo "pwd"
pwd
echo "ls -l"
ls -l
echo "ls -l $PCRDATADIR/$MIDASFILE"
ls -l $PCRDATADIR/$MIDASFILE
echo "ls -l ../mu3eTrirec/trirec.conf"
ls -l ../mu3eTrirec/trirec.conf
echo "ls -l test.sim0.root"
ls -l test.sim0.root

date

echo "../_build/mu3eTrirec/mu3eTrirec $ANLZR --input test.sim0.root --input-mid $PCRDATADIR/$MIDASFILE --conf ../mu3eTrirec/trirec.conf --output ./$ROOTFILE"
../_build/mu3eTrirec/mu3eTrirec $ANLZR --input test.sim0.root --input-mid $PCRDATADIR/$MIDASFILE --conf ../mu3eTrirec/trirec.conf --output ./$ROOTFILE

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

date

# BATCH END

date
echo "run: This is the end, my friend"
