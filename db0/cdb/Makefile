ROOTCINT      = rootcling
ROOTCFLAGS    = $(shell root-config --cflags)
ROOTGLIBS     = $(shell root-config --glibs)
ROOTLDFLAGS   = $(shell root-config --ldflags)

INCMONGODB    = -I/usr/local/include/mongocxx/v_noabi
INCMONGODB   += -I/usr/local/include/bsoncxx/v_noabi
LIBMONGODB    = -lmongocxx -lbsoncxx

LIBBOOST      = -L/usr/lib64 -lboost_serialization

EXTHEADERS    = -I../..
UTIL          = ../../util

#CXX          := /usr/bin/c++

CXXFLAGS     := -std=c++1z -g -O0 -Wall -fPIC -pipe  #CXXFLAGS     += $(ROOTCFLAGS)
CXXFLAGS     += $(ROOTCFLAGS)

LD           := $(CXX)
LDFLAGS       = -g -O0  #?? -Wl -rpath$(ROOTSYS)
SOFLAGS       = -g -shared  #?? -Wl -rpath

GLIBS         = $(filter-out -lz, $(ROOTGLIBS))


# -- CDB (Conditions DB) files
FILES = cdb.o cdbAscii.o cdbMongo.o cdbJSON.o \
        calAbs.o calPixel.o calPixelAlignment.o \
        CdbClassFactory.o payload.o 


# -- Default rules
$(addprefix obj/,%.o) : %.cc %.hh
	$(CXX) $(CXXFLAGS) $(INCMONGODB) $(EXTHEADERS) -c $< -o $@

$(addprefix obj/,%.o) : %.cc
	$(CXX) $(CXXFLAGS) $(INCMONGODB) $(EXTHEADERS) -c $< -o $@


all: prep lib bin

lib: $(addprefix obj/,$(FILES) )
	$(CXX) $(SOFLAGS) $(addprefix obj/,$(FILES)) -o lib/libCDB.so $(UTIL)/lib/libAnaUtil.so $(GLIBS) 


bin: lib obj/testRun.o obj/testCDB.o obj/fillDB.o obj/serializePixelAlignment.o
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/testRun.o -o bin/testRun $(LIBMONGODB) $(GLIBS) $(UTIL)/lib/libAnaUtil.so lib/libCDB.so 
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/testCDB.o -o bin/testCDB $(LIBMONGODB) $(GLIBS) $(UTIL)/lib/libAnaUtil.so lib/libCDB.so 
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/fillDB.o -o bin/fillDB $(LIBMONGODB) $(GLIBS) $(UTIL)/lib/libAnaUtil.so
	$(CXX) $(LDFLAGS) -DNDEBUG -rdynamic obj/serializePixelAlignment.o -o bin/spa $(LIBMONGODB) $(LIBBOOST) $(GLIBS) $(UTIL)/lib/libAnaUtil.so


clean:
	rm -f obj/*.o lib/*.so bin/testCDB

# -- preparatory setup
prep:
	mkdir -p obj bin lib
	cd lib && ln -f -s ../$(UTIL)/lib/libAnaUtil.so && cd -

