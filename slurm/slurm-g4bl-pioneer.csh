#! /bin/csh -f

#
#SBATCH -e
#SBATCH -o

setenv JOB 
setenv STEERFILE    $JOB.i
setenv ROOTFILENAME $JOB.root
setenv MOMENTUM     
setenv SETTINGS     
setenv STORAGE1
setenv SITE
setenv G4BLOUTPUTDIR
setenv G4BLTRACKFILE

# -- set up env required for running
source /psi/home/langenegger/data/g4bl/root-062206/bin/thisroot.csh
source /psi/home/langenegger/data/g4bl/geant4.10.05.p01/bin/geant4.csh /psi/home/langenegger/data/g4bl/geant4.10.05.p01/bin

/opt/psi/Tools/Pmodules/1.0.0rc10/bin/modulecmd tcsh -s load Qt/5.12.10

# -- g4bl stuff
setenv GEANT4_DIR /psi/home/langenegger/data/g4bl/geant4.10.05.p01
setenv G4BL_DIR /psi/home/langenegger/data/g4bl/G4beamline-3.06
setenv G4BLPIONEER /psi/home/langenegger/data/g4bl/pioneer-g4bl
setenv PATH ${PATH}:${G4BL_DIR}/bin

setenv PATH ${PATH}:/opt/psi/Programming/Qt/5.12.10/5.12.10/gcc_64/bin/
setenv LD_LIBRARY_PATH /opt/psi/Programming/Qt/5.12.10/5.12.10/gcc_64/lib:${LD_LIBRARY_PATH}


echo "=========================="
echo "====> SLURM  wrapper <===="
echo "=========================="
date

echo "--> Running SLURM pioneer g4bl job wrapper"

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
# -- Run g4bl
# ----------------------------------------------------------------------
echo "--> Run g4bl for job ->$JOB<-"
echo "pwd"
pwd
echo "ls -l"
ls -l

# PIONEER_SlantedTgtE_prod-40009.i histoFile=PIONEER_SlantedTgtE_prod-40009.root
echo "----------------------------------------------------------------------"
echo "g4bl $STEERFILE histoFile=$ROOTFILENAME momentum=$MOMENTUM $SETTINGS"
echo "----------------------------------------------------------------------"
g4bl $STEERFILE histoFile=$ROOTFILENAME  momentum=$MOMENTUM $SETTINGS |& tee g4bl.log
date
ls -rtl

echo "slurm check size of rootfile produced"
ls -l ./$JOB.root

mkdir -p $STORAGE1

# -- g4bl production
if ( -e ./$JOB.root ) then
    cp ./$JOB.root $STORAGE1
    setenv BLA  `ls -l $STORAGE1/$JOB.root`
    echo "slurm check that rootfile was copied $BLA"
    ls -l $STORAGE1/$JOB.root
endif

# -- g4bl transport
if ( -e ./CALOCNTR.txt ) then
    # dummy entry for monSlurm
    cp ./CALOCNTR.txt $STORAGE1/$JOB.root
    cp ./CALOCNTR.txt $STORAGE1/$JOB-CALOCNTR.txt
    cp ./CALOENTR.txt $STORAGE1/$JOB-CALOENTR.txt
    cp ./g4bl.log $STORAGE1/$JOB-g4bl.log

    if ( -e ./profile.txt ) then
       cp ./profile.txt $STORAGE1/$JOB-profile.txt
    endif
    if ( -e ./profile-13.txt ) then
       cp ./profile-13.txt $STORAGE1/$JOB-profile-13.txt
    endif
    if ( -e ./profile-211.txt ) then
       cp ./profile-211.txt $STORAGE1/$JOB-profile-211.txt
    endif
    if ( -e ./profile-11.txt ) then
       cp ./profile-11.txt $STORAGE1/$JOB-profile-11.txt
    endif
    if ( -e ./profile-2212.txt ) then
       cp ./profile-2212.txt $STORAGE1/$JOB-profile-2212.txt
    endif
    if ( -e ./profile-0.txt ) then
       cp ./profile-0.txt $STORAGE1/$JOB-profile-0.txt
    endif
else
    echo "no output files to copy"
endif
    

date

# BATCH END

date
echo "run: This is the end, my friend"
