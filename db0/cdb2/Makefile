ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

LHOST = $(shell hostname)
LDIRECTORY = $(shell pwd)

# -- ARGH!? But it works ...
ifneq (,$(findstring merlin,$(LHOST)))
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

EXTHEADERS    = -I../..

#CXX          := /usr/bin/c++

CXXFLAGS     := -std=c++1z -g -O0 -Wall -fPIC -pipe  #CXXFLAGS     += $(ROOTCFLAGS)
CXXFLAGS     += $(ROOTCFLAGS)

LD           := $(CXX)
LDFLAGS      := -g -O0  #?? -Wl -rpath$(ROOTSYS)
SOFLAGS      += -g -shared  #?? -Wl -rpath

GLIBS         = $(filter-out -lz, $(ROOTGLIBS))


OSUFFIX := .o

# -- CDB (Conditions DB) files
LFILES = cdbAbs.o cdbMongo.o cdbJSON.o cdbRest.o \
         calAbs.o \
         calPixelAlignment.o calFibreAlignment.o  \
         calMppcAlignment.o calTileAlignment.o \
         calPixelQuality.o calPixelQualityV.o calPixelQualityM.o \
         calPixelCablingMap.o calDetConfV1.o \
         Mu3eConditions.o Mu3eCalFactory.o \
         base64.o \
         payload.o cfgPayload.o cdbUtil.o runRecord.o

LBINARIES = testCDB fillDB \
            syncCloud syncMongo \
            testPixelQuality pcrPixelQuality \
            cdbInitDB cdbPrintPayload  \
            cdbPrintConfig \
            uploadDetConfig downloadDetConfig 

ifneq (,$(findstring merlin,$(LHOST)))
       $(info ************  MERLIN VERSION ************)
       FILES = $(filter-out cdbMongo.o, $(LFILES))
       BINARIES = $(filter-out testCDB fillDB syncCloud syncMongo, $(LBINARIES))
else
       $(info ************  non-MERLIN VERSION ************)
       FILES = $(LFILES)
       BINARIES = $(LBINARIES)
endif



# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\" $(INCMONGODB) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) -DLOCALDIR=\"$(LDIRECTORY)\" $(INCMONGODB) $(EXTHEADERS) -c $< -o $@

$(addprefix bin/,%) : $(addprefix obj/,%.o)
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic $< -o $@ $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so


# -- targets
all: prep lib bin

lib: lib/libCDB.so

bin: $(addprefix bin/,$(BINARIES) )

lib/libCDB.so: $(addprefix obj/,$(FILES) )
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libCDB.so $(GLIBS)


clean:
	rm -f obj/*.o lib/*.so bin/*

# -- preparatory setup
prep:
	mkdir -p obj bin lib
