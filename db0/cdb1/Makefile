ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

LHOST = $(shell hostname)

ifneq (,$(findstring merlin,$(LHOST)))
  INCMONGODB    = -I/psi/home/langenegger/data/mongo/install/include/mongocxx/v_noabi
  INCMONGODB   += -I/psi/home/langenegger/data/mongo/install/include/bsoncxx/v_noabi
  LIBMONGODB    = -L /psi/home/langenegger/data/mongo/install/lib64 -lmongocxx -lbsoncxx
  LIBCURL       = -L/usr/lib64 -lcurl
  LIBBOOST      = -L$(BOOST_LIBRARY_DIR) -lboost_serialization
else
  INCMONGODB    = -I/usr/local/include/mongocxx/v_noabi
  INCMONGODB   += -I/usr/local/include/bsoncxx/v_noabi
  LIBMONGODB    = -lmongocxx -lbsoncxx
  LIBCURL       = -L/usr/lib64 -lcurl
  LIBBOOST      = -L/usr/lib64 -lboost_serialization
endif

EXTHEADERS    = -I../..

#CXX          := /usr/bin/c++

CXXFLAGS     := -std=c++1z -g -O0 -Wall -fPIC -pipe  #CXXFLAGS     += $(ROOTCFLAGS)
CXXFLAGS     += $(ROOTCFLAGS)

LD           := $(CXX)
LDFLAGS       = -g -O0  #?? -Wl -rpath$(ROOTSYS)
SOFLAGS       = -g -shared  #?? -Wl -rpath

GLIBS         = $(filter-out -lz, $(ROOTGLIBS))


# -- CDB (Conditions DB) files
FILES = cdbAbs.o cdbMongo.o cdbJSON.o cdbRest.o cdbROOT.o \
        calAbs.o calPixelAlignment.o \
        Mu3eConditions.o  \
        base64.o
        payload.o cdbUtil.o


# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(INCMONGODB) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(INCMONGODB) $(EXTHEADERS) -c $< -o $@


all: prep lib bin

lib: $(addprefix obj/,$(FILES) )
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libCDB.so $(GLIBS) 


bin: lib obj/testCDB.o obj/fillDB.o obj/syncCloud.o 
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/testCDB.o -o bin/testCDB $(LIBMONGODB) $(LIBCURL) $(GLIBS) lib/libCDB.so 
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/fillDB.o -o bin/fillDB $(LIBMONGODB) $(LIBCURL) $(GLIBS)  lib/libCDB.so 
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/syncCloud.o -o bin/syncCloud $(LIBMONGODB) $(LIBCURL) $(GLIBS)  

clean:
	rm -f obj/*.o lib/*.so bin/testCDB bin/fillDB

# -- preparatory setup
prep:
	mkdir -p obj bin lib

