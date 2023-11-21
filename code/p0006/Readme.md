# CDB Introduction

Note: CDB = conddb. There is no danger of confusion because there is no configuration DB, only a conditions DB.


## Global tags and tags

| Global Tag | Release tag | Tags contained | Remarks |
| -----------|-------------|----------------|----------|
| mcidealhead  |  [dev](https://bitbucket.org/mu3e/mu3e/src/dev/)  | pixelalignment, fibrealignment, mppcalignment, tilealignment   |  |
| mcidealv5.0  | [v5.0](https://bitbucket.org/mu3e/mu3e/src/v5.0/)  | pixelalignment, fibrealignment, mppcalignment, tilealignment    | untested |


## Status
*  [conddb](https://bitbucket.org/mu3e/mu3e/src/ursl-cdb/conddb/): filled with code, compiles, links, and runs
    *  [conddb/CMakeLists.txt](https://bitbucket.org/mu3e/mu3e/src/ursl-cdb/conddb/CMakeLists.txt)
    *  and all the rest of the CDB code
*  [mu3eTrirec](https://bitbucket.org/mu3e/mu3e/src/ursl-cdb/mu3eTrirec) with "option" to initialize alignment information from CDB: filled with code, compiles, links, and runs. FIRST version.
    *  [mu3eTrirec/trirec.cpp](https://bitbucket.org/mu3e/mu3e/src/8a75a701c31aa3406af658c05f9afd8715e369e2/mu3eTrirec/trirec.cpp#lines-63:80)
    *  [mu3eTrirec/src/mu3e/rec/trirec.h](https://bitbucket.org/mu3e/mu3e/src/8a75a701c31aa3406af658c05f9afd8715e369e2/mu3eTrirec/src/mu3e/rec/trirec.h#lines-150:152)
    *  [mu3eTrirec/src/mu3e/rec/util/root.hpp](https://bitbucket.org/mu3e/mu3e/src/0ec59e53df7b6d8e848e9831693ab641d73ee24b/mu3eTrirec/src/mu3e/rec/util/root.hpp#lines-54)
    *  [mu3eTrirec/src/mu3e/rec/Conf.h](https://bitbucket.org/mu3e/mu3e/src/0ec59e53df7b6d8e848e9831693ab641d73ee24b/mu3eTrirec/src/mu3e/rec/Conf.h#lines-65:72)
    *  [mu3eTrirec/src/mu3e/rec/Conf.cpp](https://bitbucket.org/mu3e/mu3e/src/0ec59e53df7b6d8e848e9831693ab641d73ee24b/mu3eTrirec/src/mu3e/rec/Conf.cpp#lines-106)
    *  [mu3eTrirec/src/mu3e/rec/SiDet.cpp](https://bitbucket.org/mu3e/mu3e/src/ursl-cdb/mu3eTrirec/src/mu3e/rec/SiDet.cpp)
    *  [mu3eTrirec/src/mu3e/rec/TlDet.cpp](https://bitbucket.org/mu3e/mu3e/src/ursl-cdb/mu3eTrirec/src/mu3e/rec/TlDet.cpp)
    *  [mu3eTrirec/src/mu3e/rec/FbDet.cpp](https://bitbucket.org/mu3e/mu3e/src/ursl-cdb/mu3eTrirec/src/mu3e/rec/FbDet.cpp)
*  mu3eSim: TODO

## Tutorial
The following should work on merlin.psi.ch. For other places you need to change the bsoncxx/mongocxx/mongoc locations in [conddb/CMakeLists.txt](https://bitbucket.org/mu3e/mu3e/src/ursl-cdb/conddb/CMakeLists.txt)
```
git clone git@bitbucket.org:mu3e/mu3e mu3e-cdb
cd mu3e-cdb
git checkout ursl-cdb
git submodule update --init --recursive
mkdir _build && cd _build
cmake -DMU3E_CONDDB=ON ..
make -j20
cd ..
```

## Starting the CDB
Set up the JSON version of the CDB:
```
cd conddb/ascii
../../_build/conddb/test/cdbInitDB -j /data/experiment/mu3e/code/cdb/json -m all
```
Check that a payload: 
```
cd conddb/ascii
../../_build/conddb/test/cdbPrintPayload /data/experiment/mu3e/code/cdb/json/payloads/tag_pixelalignment_mcidealhead_iov_1
```

## Running trirec
```
cd run
../_build/mu3eTrirec/mu3eTrirec directory/mu3e_sorted_000779.root directory/mu3e_trirec_000779.root
```

This results in 
```
merlin-l-002>date; 
Tue Nov 21 10:45:17 CET 2023
merlin-l-002>../_build/mu3eTrirec/mu3eTrirec directory/mu3e_sorted_000779.root directory/mu3e_trirec_000779.root
D [read_conf_internal] read_conf '[].json'
D [read_conf_internal] read_conf '/data/user/langenegger/mu3e-cdb/run/trirec.conf'
  fb.length = 0 <= 287.75
  fb.diameter = 0 <= 0.25
  fb.deadWidth = 0 <= 0.005
  fb.nLayers = 0 <= 3
  fb.nColumns = 0 <= 126
  fb.nRibbons = 0 <= 12
  fb.refidx = 0 <= 1.59
  tl.r_outer = 0 <= 62.5
  tl.r_inner = 0 <= 47
  tl.nPhi = 0 <= 56
  tl.nZ = 0 <= 52
  tl.nModules = 0 <= 7
  tl.resolution = 0 <= 0.06
  si.n_fine_bits = 0 <= 4
  tl.n_fine_bits = 0 <= 10
  fb.n_fine_bits = 0 <= 10
initialize conddb
Mu3eConditions::Mu3eConditions(mcidealhead, JSON)
cdbJSON::readGlobalTags() from  gtdir = /data/experiment/mu3e/code/cdb/json/globaltags
Mu3eConditions::registerCalibration> name ->pixelalignment_<- with tag ->pixelalignment_mcidealhead<- ...  done
Mu3eConditions::registerCalibration> name ->fibrealignment_<- with tag ->fibrealignment_mcidealhead<- ...  done
Mu3eConditions::registerCalibration> name ->mppcalignment_<- with tag ->mppcalignment_mcidealhead<- ...  done
Mu3eConditions::registerCalibration> name ->tilealignment_<- with tag ->tilealignment_mcidealhead<- ...  done
cdbJSON::getPayload() Read /data/experiment/mu3e/code/cdb/json/payloads/tag_fibrealignment_mcidealhead_iov_1
calFibreAlignment::calculate() with fHash ->tag_fibrealignment_mcidealhead_iov_1<-  header: deadface inserted 4524 constants
cdbJSON::getPayload() Read /data/experiment/mu3e/code/cdb/json/payloads/tag_mppcalignment_mcidealhead_iov_1
calMppcAlignment::calculate() with fHash ->tag_mppcalignment_mcidealhead_iov_1<- header: deadface inserted 24 constants
cdbJSON::getPayload() Read /data/experiment/mu3e/code/cdb/json/payloads/tag_pixelalignment_mcidealhead_iov_1
calPixelAlignment::calculate() with fHash ->tag_pixelalignment_mcidealhead_iov_1<- header: deadface inserted 2844 constants
cdbJSON::getPayload() Read /data/experiment/mu3e/code/cdb/json/payloads/tag_tilealignment_mcidealhead_iov_1
calTileAlignment::calculate() with fHash ->tag_tilealignment_mcidealhead_iov_1<- header: deadface inserted 5824 constants
filling SiDet from CDB with pixelalignment_ cal = 0x34931d0
I [SiDet.cpp:306,init] init global variables
  layer 0 : 8 ladders, r = 23.1, l = 41.5
  layer 1 : 10 ladders, r = 29.6, l = 41.5
  layer 2 : 24 ladders, r = 72.0, l = 557.9
  layer 3 : 28 ladders, r = 85.0, l = 568.3
filling FbDet from CDB with fibrealignment_ cal = 0x3493010
filling MPPC in FbDet from CDB with mppcalignment_ cal = 0x2fe4a40
I [FbDet::init] init global vars
  Array readout
  12 mppcs per side
  r = 62.7 mm
filling TlDet from CDB with tilealignment_ cal = 0x3049180
W [branch_t] branch 'fibreasic_mc_i' not found
W [branch_t] branch 'fibreasic_mc_n' not found
W [branch_t] branch 'fibreasic_id' not found
W [branch_t] branch 'fibreasic_timestamp' not found
W [branch_t] branch 'fibreasic_time' not found
W [branch_t] branch 'pos_g_x' not found
W [branch_t] branch 'pos_g_y' not found
W [branch_t] branch 'pos_g_z' not found
W [branch_t] branch 'pos_l_x' not found
W [branch_t] branch 'pos_l_y' not found
W [branch_t] branch 'pos_l_z' not found
W [branch_t] branch 'p_in_x' not found
W [branch_t] branch 'p_in_y' not found
W [branch_t] branch 'p_in_z' not found
W [branch_t] branch 'e_in' not found
W [branch_t] branch 'p_out_x' not found
W [branch_t] branch 'p_out_y' not found
W [branch_t] branch 'p_out_z' not found
W [branch_t] branch 'e_out' not found
## FRAME 840 ##
I [trirec.h:266,run] trec = 4.7 seconds / 841 frames
I [trirec.h:267,run] twrite = 0.4 seconds
D [RootFile::Write] write config
merlin-l-002> 
```