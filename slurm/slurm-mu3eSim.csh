#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o

setenv JOB 
setenv STORAGE1
setenv SITE

# -- set up env required for running
source /data/experiment/mu3e/code/external/root/root6-22-06/install/bin/thisroot.csh
source /psi/home/langenegger/data/g4-install/bin/geant4.csh /psi/home/langenegger/data/g4-install/bin
#
/opt/psi/Tools/Pmodules/1.0.0rc10/bin/modulecmd tcsh -s load Qt/5.12.10
#
setenv  ROOT_DIR "/data/experiment/mu3e/code/external/root/root6-22-06/install"
setenv  CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}:/data/experiment/mu3e/code/external/Eigen3/eigen3-3-9:/psi/home/langenegger/data/g4-install"
#
setenv BOOST_ROOT "/opt/psi/Compiler/boost/1.73.0/gcc/10.1.0/"
setenv EIGEN3_INCLUDE_DIR /data/experiment/mu3e/code/external/Eigen3/eigen3-3-9
#
setenv PATH ${PATH}:/opt/psi/Programming/Qt/5.12.10/5.12.10/gcc_64/bin/
setenv LD_LIBRARY_PATH /opt/psi/Programming/Qt/5.12.10/5.12.10/gcc_64/lib:${LD_LIBRARY_PATH}


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
echo "--> check visibility of /psi/home/langenegger/data/."
ls -l /psi/home/langenegger/data/.
echo "--> End of env testing"

# BATCH START


# ----------------------------------------------------------------------
# -- setup runtime directory
# ----------------------------------------------------------------------
echo "--> Extract tar file"
date
tar zxf ./$JOB.tar.gz
cd run
cp ../$JOB.mac .

# ----------------------------------------------------------------------
# -- Run mu3eSim
# ----------------------------------------------------------------------
echo "--> Run mu3eSim"
echo "pwd"
pwd
echo "ls -l"
ls -l

echo "../_build/mu3eSim/mu3eSim --script $JOB.mac --output ./$JOB.root"
../_build/mu3eSim/mu3eSim --script $JOB.mac --output ./$JOB.root
date
ls -rtl
echo "slurm check size of rootfile produced"
ls -l ./$JOB.root

cp ./$JOB.root $STORAGE1
setenv BLA  `ls -l $STORAGE1/$JOB.root`
echo "slurm check that rootfile was copied $BLA"
ls -l $STORAGE1/$JOB.root

date

# BATCH END

date
echo "run: This is the end, my friend"
