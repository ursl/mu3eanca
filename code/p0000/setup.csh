#set verbose

source /data/experiment/mu3e/code/external/root/root6-22-06/install/bin/thisroot.csh
source /psi/home/langenegger/data/g4-install/bin/geant4.csh /psi/home/langenegger/data/g4-install/bin

setenv G4INC /psi/home/langenegger/data/g4-install/include/Geant4/

module use unstable
#module load gcc/10.1.0 boost/1.73.0
#module load gcc/10.3.0  eigen/3.3.9
module load gcc/10.3.0 
module load boost/1.76.0
module load git/2.33.1
module load cmake/3.19.2
module load cuda/11.2.2

# -- "-s" suppress the warning about unstable Qt version
module -s load Qt/5.12.10

setenv  ROOT_DIR "/data/experiment/mu3e/code/external/root/root6-22-06/install"
setenv  CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}:/data/experiment/mu3e/code/external/Eigen3/eigen3-3-9:/psi/home/langenegger/data/g4-install:/psi/home/langenegger/data/fmt-8.1.1-install/lib64/cmake/fmt"
#setenv  CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}:/psi/home/langenegger/data/g4-install"

#setenv BOOST_ROOT "/opt/psi/Compiler/boost/1.73.0/gcc/10.1.0/"

#setenv Eigen3_DIR ${EIGEN_DIR}
setenv Boost_DIR ${BOOST_DIR}

setenv fmt_DIR /psi/home/langenegger/data/fmt-9.0.0-install

# qt5
setenv PATH ${PATH}:/opt/psi/Programming/Qt/5.12.10/5.12.10/gcc_64/bin/
setenv LD_LIBRARY_PATH /opt/psi/Programming/Qt/5.12.10/5.12.10/gcc_64/lib:${LD_LIBRARY_PATH}

# -- midas -> set_env.csh
#setenv MIDASSYS /psi/home/langenegger/mu3e/midas
#setenv MIDAS_EXPTAB ${MIDASSYS}/online/exptab
#
#setenv CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}:${MIDASSYS}
#setenv PATH ${PATH}:$MIDASSYS/bin
