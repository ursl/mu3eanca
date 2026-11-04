#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o
#SBATCH --mem=8G

setenv JOB
setenv RUN
setenv EXE
setenv MIDASFILE
setenv STORAGE1

echo "====================================================="
echo "====> SLURM RUN2026 prompt0 midasmeta wrapper    <===="
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

# mu3e_midas_meta writes <input>.json next to the MIDAS file.
echo "--> Run mu3e_midas_meta"
echo "ls -l $MIDASFILE"
ls -l $MIDASFILE

echo "$EXE $MIDASFILE"
$EXE $MIDASFILE
if ( $status != 0 ) then
    echo "mu3e_midas_meta failed"
    exit 1
endif

set JSON = ${MIDASFILE}.json
ls -l $JSON

echo "cp $JSON $STORAGE1/$RUN/"
cp $JSON $STORAGE1/$RUN/
if ( $status != 0 ) then
    echo "copy of json failed"
    exit 1
endif
echo "slurm check that json was copied to $STORAGE1/$RUN"
ls -l $STORAGE1/$RUN

date

# BATCH END

date
echo "run: This is the end, my friend"
