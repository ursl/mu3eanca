module unload gcc cmake
module load gcc/12.3.0 cmake/3.26.3 Python/3.13.2

setenv SW "/data/experiment/mu3e/code/software"

setenv Boost_DIR ${SW}
setenv Eigen3_DIR ${SW}
setenv LZ4_ROOT ${SW}

setenv PATH ${SW}/bin:${PATH}
setenv LD_LIBRARY_PATH ${SW}/lib:${LD_LIBRARY_PATH}

setenv Geant4_ROOT ${SW}
setenv Geant4_PREFIX `geant4-config --prefix`
setenv ROOT_ROOT ${SW}
setenv ROOTSYS ${SW}

setenv CDB "/data/experiment/mu3e/cdb/"

# midas
setenv MIDASSYS ${SW}/src/midas

setenv CMAKE_PREFIX_PATH ${MIDASSYS}:/data/experiment/mu3e/code/software/src/mu3e_dev/install:/data/experiment/mu3e/code/software/src/midas:/data/experiment/mu3e/code/software_gcc-12.3.0:/data/experiment/mu3e/code/external/Eigen3/eigen3-3-9/install:/data/experiment/mu3e/code/external/geant/geant4.10.07

setenv PATH ${MIDASSYS}/bin:$PATH
setenv LD_LIBRARY_PATH ${MIDASSYS}/lib:${LD_LIBRARY_PATH}

# -- conda for snakemake
source  /data/experiment/mu3e/code/conda/etc/profile.d/conda.csh

