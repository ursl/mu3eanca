#!/bin/csh

module load gcc/14.2.0

setenv LD_LIBRARY_PATH "/data/user/langenegger/mu3e/util/lib"

setenv SW "/data/project/mu3e/software"

setenv Boost_DIR "${SW}/boost-1.75.0"
setenv Eigen3_DIR "${SW}/eigen-3.4.0"
setenv LZ4_ROOT "${SW}/lz4-1.10.0"
setenv LD_LIBRARY_PATH "${Boost_DIR}/lib:${LZ4_ROOT}/lib:${LD_LIBRARY_PATH}"

setenv ROOTSYS "${SW}/root-v6-29-02"
setenv PATH "${ROOTSYS}/bin:${PATH}"
setenv LD_LIBRARY_PATH "${ROOTSYS}/lib:${LD_LIBRARY_PATH}"
source "${SW}/geant4-v11.1.2/bin/geant4.csh" ${SW}/geant4-v11.1.2/bin

echo "Boost_DIR=$Boost_DIR"
echo "Eigen3_DIR=$Eigen3_DIR"
echo "LZ4_ROOT=$LZ4_ROOT"
echo "ROOTSYS=$ROOTSYS"
echo "Geant4_PREFIX=`geant4-config --prefix`"


# mu3e software
if (! $?MU3E_TAG) then
    setenv MU3E_TAG "dev"
endif
if ("$MU3E_TAG" != "-") then
    setenv MU3E_PREFIX "${SW}/mu3e_$MU3E_TAG"
    echo "MU3E_PREFIX=$MU3E_PREFIX"
    if ($?CMAKE_PREFIX_PATH) then
        setenv CMAKE_PREFIX_PATH "${MU3E_PREFIX}:${CMAKE_PREFIX_PATH}"
    else
        setenv CMAKE_PREFIX_PATH "${MU3E_PREFIX}"
    endif
    setenv PATH "${MU3E_PREFIX}/bin:${PATH}"
    setenv LD_LIBRARY_PATH "${MU3E_PREFIX}/lib64:${LD_LIBRARY_PATH}"
endif
