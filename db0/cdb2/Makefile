ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

LHOST = $(shell hostname)

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


# -- CDB (Conditions DB) files
LFILES = cdbAbs.o cdbMongo.o cdbJSON.o cdbRest.o \
         calAbs.o \
         calPixelAlignment.o calFibreAlignment.o  \
         calMppcAlignment.o calTileAlignment.o \
         calPixelQuality.o calPixelQualityV.o calPixelQualityM.o \
         calPixelCablingMap.o \
         Mu3eConditions.o Mu3eCalFactory.o \
         base64.o \
         payload.o cfgPayload.o cdbUtil.o runRecord.o

LBINARIES = obj/testCDB.o obj/fillDB.o \
            obj/syncCloud.o obj/syncMongo.o \
            obj/testPixelQuality.o obj/pcrPixelQuality.o \
            obj/cdbInitDB.o obj/cdbPrintPayload.o  \
            obj/cdbPrintConfig.o


ifneq (,$(findstring merlin,$(LHOST)))
       $(info ************  MERLIN VERSION ************)
       FILES = $(filter-out cdbMongo.o, $(LFILES))
       BINARIES = $(filter-out testCDB.o fillDB.o syncCloud.o syncMongo.o, $(LBINARIES))
else
       FILES = LFILES
endif



# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(INCMONGODB) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(INCMONGODB) $(EXTHEADERS) -c $< -o $@


# -- targets
all: prep lib bin

lib: $(addprefix obj/,$(FILES) )
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libCDB.so $(GLIBS)

bin: lib
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/cdbPrintPayload.o -o bin/cdbPrintPayload $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/cdbPrintConfig.o -o bin/cdbPrintConfig $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/cdbInitDB.o -o bin/cdbInitDB $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/pcrPixelQuality.o -o bin/pcrPixelQuality $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/testPixelQuality.o -o bin/testPixelQuality $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/testCDB.o -o bin/testCDB $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/fillDB.o -o bin/fillDB $(LIBMONGODB) $(LIBCURL) $(GLIBS)  lib/libCDB.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/syncCloud.o -o bin/syncCloud $(LIBMONGODB) $(LIBCURL) $(GLIBS)
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/syncMongo.o -o bin/syncMongo $(LIBMONGODB) $(LIBCURL) $(GLIBS)  lib/libCDB.so

clean:
	rm -f obj/*.o lib/*.so bin/*

# -- preparatory setup
prep:
	mkdir -p obj bin lib
