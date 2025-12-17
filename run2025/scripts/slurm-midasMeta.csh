#! /bin/csh -f

#SBATCH --job-name=midasMetaData      # Job name

setenv DIR 
setenv MIDASFILE 

cd $DIR
/data/project/mu3e/offline/251215-midasMeta2/mu3eUtil/_build/tools/midasMeta/mu3e_midas_meta $DIR/$MIDASFILE

