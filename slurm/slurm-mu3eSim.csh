#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o

#setenv GENRELEASE CMSSW_10_2_3

source /psi/home/langenegger/mu3e/setup.csh

echo "=========================="
echo "====> SLURM  wrapper <===="
echo "=========================="
date

echo "--> Running SLURM mu3eSim job wrapper"

# ----------------------------------------------------------------------
# -- The Basics
# ----------------------------------------------------------------------
echo "--> Environment"
hostname
uname -a
limit coredumpsize 0

pwd
echo "--> End of env testing"

# BATCH START



date

# BATCH END

date
echo "run: This is the end, my friend"
