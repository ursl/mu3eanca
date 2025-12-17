#! /bin/csh -f                                                                                                                                                                                                                  

setenv JOB
setenv DIR

cd $DIR
pwd
echo $DIR/run0$JOB.mid.lz4
/data/project/mu3e/offline/251215-midasMeta2/mu3eUtil/_build/tools/midasMeta/mu3e_midas_meta $DIR/run0$JOB.mid.lz4
