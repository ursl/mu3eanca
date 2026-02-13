# ======================================================================
# NOTE: make all
# -----
#
# ======================================================================
.PHONY: prep clean vars

# -- create a link to an external dependency (https://github.com/ursl/util)
#    no submodule because of constant changes in util
UTIL := ../../util
CDB  := ../../db0/cdb2

ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

# -- ARGH!? But it works ...
ifneq (,$(findstring merlin,$(LHOST)))
  INCMONGODB    =
  LIBMONGODB    =
  LIBCURL       = -L/usr/lib64 -lcurl
  LIBBOOST      = -L$(BOOST_LIBRARY_DIR) -lboost_serialization
  SOFLAGS       =
else ifneq (,$(findstring mu3ebe,$(LHOST)))
  INCMONGODB    =
  LIBMONGODB    =
  LIBCURL       = -L/usr/lib64 -lcurl
  LIBBOOST      = -L$(BOOST_LIBRARY_DIR) -lboost_serialization
  SOFLAGS       =
else ifneq (,$(findstring moor,$(LHOST)))
  INCMONGODB    = -I/usr/local/include/mongocxx/v_noabi
  INCMONGODB   += -I/usr/local/include/bsoncxx/v_noabi
  INCMONGODB   += -I/usr/local/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc
  LIBMONGODB    = -L/usr/local/lib -lmongocxx -lbsoncxx
  LIBCURL       = -L/opt/homebrew/lib -lcurl
  LIBBOOST      = -L/usr/lib64 -lboost_serialization
  SOFLAGS       = $(LIBMONGODB) $(LIBCURL)
else ifneq (,$(findstring mu3ebe,$(LHOST)))
  INCMONGODB    = -I/home/mu3e/.ursl/mongo-cxx-driver-r3.9.0-install/include/mongocxx/v_noabi
  INCMONGODB   += -I/home/mu3e/.ursl/mongo-cxx-driver-r3.9.0-install/include/bsoncxx/v_noabi
  INCMONGODB   += -I/home/mu3e/.ursl/include
  LIBMONGODB    = -L/home/mu3e/.ursl/mongo-cxx-driver-r3.9.0-install/lib64 -lmongocxx -lbsoncxx
  LIBCURL       = -L/home/mu3e/.ursl/lib64 -lcurl
  LIBBOOST      = -L/usr/lib64 -lboost_serialization
  SOFLAGS       = $(LIBMONGODB) $(LIBCURL)
else
  INCMONGODB    = -I/usr/local/include/mongocxx/v_noabi
  INCMONGODB   += -I/usr/local/include/bsoncxx/v_noabi
  LIBMONGODB    = -lmongocxx -lbsoncxx
  LIBCURL       = -L/usr/lib64 -lcurl
  LIBBOOST      = -L/usr/lib64 -lboost_serialization
  SOFLAGS       =
endif

CXXFLAGS      = -std=c++1z -g -O0 -Wall -fPIC -pipe
LD            = $(CXX)
LDFLAGS       = -g -pthread #?? -Wl -rpath$(ROOTSYS)
SOFLAGS       = -g -shared  #?? -Wl -rpath

CXXFLAGS     += $(ROOTCFLAGS)
GLIBS         = $(filter-out -lz, $(ROOTGLIBS)) -lTMVA -lTMVAGui -lMinuit 
EXTHEADERS    = -I../../util -I..  -I$(CDB)

OTHERLIBS 	  = $(UTIL)/lib/libAnaUtil.so $(CDB)/lib/libCDB.so $(LIBCURL)


# -- Files
#CSTUFF =
LFILES = frameTree.o  pixelHistograms.o anaFrameTree.o plotFrameTreeResults.o anaMidasMetaTree.o
BINARIES = fillQualityPixels fillQualityTiles fillQualityFibres fillEfficiencyPixels   allHistograms \
           runPixelHistograms runAnaFrameTree runAnaMidasMetaTree runPlot \
           movieMakerVtx overlaySingleChip lvdsVsGlobalChipID produceMatrixNamesAndOffset dumpMidasMetaTree printMidasMetaFile

DICT = ${LFILES:.o=Dict.o}
DICTHEADERS = ${LFILES:.o=Dict.h}
DICTSOURCES = ${LFILES:.o=Dict.cc}

# -- Default rules
$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(EXTHEADERS) -c $< -o $@

%Dict.cc : %.hh %LinkDef.h
	$(ROOTCINT) -f $@ -c $(EXTHEADERS) $^
	mv $(subst Dict.cc,Dict_rdict.pcm,$@) lib

%Dict.cc : %.hh
	$(ROOTCINT) -f $@ -c $(EXTHEADERS) $<
	mv $(subst Dict.cc,Dict_rdict.pcm,$@) lib



$(addprefix bin/,%) : $(addprefix obj/,%.o) lib/libRun2025Analysis.so
	$(LD) $(LDFLAGS) -o $@  $<  lib/libRun2025Analysis.so $(GLIBS) $(OTHERLIBS)
 

$(addprefix lib/,%.pcm) :
	cd lib && ln -s $< && cd -


# -- Targets
all: vars prep lib bin

lib: prep $(addprefix obj/,$(CSTUFF) $(LFILES)  $(DICT))
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(LFILES) $(CSTUFF) $(DICT)) -o lib/libRun2025Analysis.so  $(GLIBS) -lMinuit $(OTHERLIBS)

bin: lib $(addprefix bin/,$(BINARIES) )


# -- preparatory setup
prep:
	mkdir -p obj bin lib
	cd lib && ln -f -s ../$(UTIL)/lib/libAnaUtil.so && cd -
	cd lib && ln -f -s ../$(CDB)/lib/libCDB.so && cd -

# -- clean up
clean:
	rm -f $(addprefix obj/,$(LFILES ) $(DICT) $(CSTUFF) runHitDataAna.o runTreeLFILES .o runPlot.o runPlotFrameTreeResults.o runMovieMakerVtx.o overlaySingleChip.o )
	rm -f $(DICTHEADERS)  $(DICTSOURCES)
	rm -f bin/*
	rm -f lib/*
	rm -f obj/*

cleanresults:
	rm -f results/*.root results/*.pdf results/*.tex results/*.tex.old
