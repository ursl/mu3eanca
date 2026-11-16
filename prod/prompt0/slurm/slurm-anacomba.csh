#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o
#SBATCH --mem=8G

setenv JOB
setenv RUN
setenv SORTEDDIR
setenv SORTEDFILE
setenv JSONFILE
setenv ROOTFILE
setenv HITS
setenv DET
setenv CLASS
setenv YAML
setenv NENTRIES
setenv SKIP
setenv GT
setenv CDB
setenv STORAGE1

echo "====================================================="
echo "====> SLURM RUN2026 prompt0 anacomba wrapper     <===="
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
cd anacomba

echo "--> Run anaComba"
echo "pwd"
pwd
echo "ls -l $SORTEDDIR/$SORTEDFILE"
ls -l $SORTEDDIR/$SORTEDFILE
echo "ls -l $JSONFILE"
ls -l $JSONFILE

# env-merlin is bash (mu3e_env.sh, modules). env-anaComba is the file
# written by install.sh. Both have to be sourced before anaComba.
bash -c 'set -e; source env-merlin && source env-anaComba && "$ANACOMBA_BIN/anaComba" -i "$SORTEDDIR/$SORTEDFILE" -o "$ROOTFILE" -m "$HITS" -d "$DET" -c "$JSONFILE" "$ANACOMBA_SCRIPTS/$YAML" -n "$NENTRIES" -s "$SKIP" -r "$CLASS" --cdb.dbconn "$CDB" --cdb.globalTag "$GT"'

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
