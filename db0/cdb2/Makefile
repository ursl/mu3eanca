ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

LHOST = $(shell hostname)
LDIRECTORY = $(shell pwd)

UTIL := ../../util

# -- ARGH!? But it works ...
ifneq (,$(or $(findstring merlin,$(LHOST)),$(findstring login001,$(LHOST)),$(findstring login002,$(LHOST))))
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
 INCMONGODB    = -I/opt/software/include/mongocxx/v_noabi
 INCMONGODB   += -I/opt/software/include/bsoncxx/v_noabi
 INCMONGODB   += -I/opt/software/include/bsoncxx/v_noabi/bsoncxx/third_party/mnmlstc
 LIBMONGODB    = -L/opt/software/lib -Wl,-rpath,/opt/software/lib -lmongocxx -lbsoncxx
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

EXTHEADERS    = -I../.. -I../../util

#CXX          := /usr/bin/c++

CXXFLAGS     := -std=c++1z -g -O0 -Wall -fPIC -pipe  #CXXFLAGS     += $(ROOTCFLAGS)
CXXFLAGS     += $(ROOTCFLAGS)

LD           := $(CXX)
LDFLAGS      := -g -O0  #?? -Wl -rpath$(ROOTSYS)
SOFLAGS      += -g -shared  #?? -Wl -rpath

GLIBS         = $(filter-out -lz, $(ROOTGLIBS))

OTHERLIBS         = $(UTIL)/lib/libAnaUtil.so 


OSUFFIX := .o

# -- CDB (Conditions DB) files  // cdbMongo.o
LFILES = cdbAbs.o  cdbJSON.o cdbRest.o calAbs.o cdbMongo.o \
         calPixelAlignment.o calFibreAlignment.o  \
         calMppcAlignment.o calTileAlignment.o \
         calPixelQualityLM.o \
         calTileQuality.o calFibreQuality.o \
         calPixelTimeCalibration.o calPixelEfficiency.o\
         calPixelCablingMap.o calDetConfV1.o calDetSetupV1.o calEventStuffV1.o\
         Mu3eConditions.o Mu3eCalFactory.o \
         base64.o \
         payload.o cfgPayload.o cdbUtil.o runRecord.o \
         RunInfo.o DataQuality.o Resource.o

LBINARIES = testCDB anaPixelQuality anaEventStuff \
            cdbInitDB cdbPrintPayload cdbInitGT cdbWritePayload \
            cdbPrintConfig \
            cdbInsertIovTag diffRunnumbers cdbPurgePayloads diffPayloads

MONGOBINARIES = fillDB syncMongo syncJSON syncRDB uploadDetConfig downloadDetConfig

ifneq (,$(or $(findstring merlin,$(LHOST)),$(findstring login001,$(LHOST)),$(findstring login002,$(LHOST))))
       $(info ************  MERLIN VERSION ************)
       FILES = $(filter-out cdbMongo.o, $(LFILES))
       BINARIES = $(filter-out fillDB syncCloud syncRDB syncMongo uploadDetConfig downloadDetConfig, $(LBINARIES))
else ifneq (,$(findstring mu3ebe,$(LHOST)))
       $(info ************  MU3EBE VERSION ************)
       FILES = $(filter-out cdbMongo.o, $(LFILES))
       BINARIES = $(filter-out uploadDetConfig downloadDetConfig testCDB fillDB syncCloud syncMongo uploadDetConfig downloadDetConfig, $(LBINARIES))
else
       $(info ************  non-MERLIN VERSION ************)
       FILES = $(LFILES)
       BINARIES = $(LBINARIES) $(MONGOBINARIES)
endif



# -- targets
all: prep lib/libCDB.so bin

# -- Build all binary object files first to avoid parallel build races
BINARY_OBJS = $(addprefix obj/,$(addsuffix .o,$(BINARIES)))
bin: $(BINARY_OBJS) $(addprefix bin/,$(BINARIES))

lib/libCDB.so: $(addprefix obj/,$(FILES))
	$(CXX) $(SOFLAGS) $^ -o $@ $(GLIBS)

# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\" $(INCMONGODB) $(EXTHEADERS) -c $< -o $@
#	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\"  $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\" $(INCMONGODB) $(EXTHEADERS) -c $< -o $@
#	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\"  $(EXTHEADERS) -c $< -o $@

# -- Binary targets: object file must exist (built by pattern rule above)
bin/% : obj/%.o lib/libCDB.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic $< -o $@ $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so $(OTHERLIBS)
#	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic $< -o $@  $(LIBCURL) $(GLIBS) lib/libCDB.so $(OTHERLIBS)

clean:
	rm -f obj/*.o lib/*.so bin/*

# -- preparatory setup
prep:
	mkdir -p obj bin lib
	cd lib && ln -f -s ../$(UTIL)/lib/libAnaUtil.so && cd -